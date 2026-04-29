#include "Arena.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <iostream>
#include <fstream>
#include <sstream>
#include <dlfcn.h>

Arena::Arena()
    : _max_rounds(0),
      _current_round(1),
      _arena_height(0),
      _arena_width(0),
      _num_mounds(0),
      _num_pits(0),
      _num_flamethrowers(0)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

bool Arena::load_config(const std::string& config_path) {
    std::ifstream input(config_path);
    if(!input){return false;}
    
    std::string line;
    while (std::getline(input, line))
    {
        if (line.empty()){continue;}
        if (!parse_config_line(line)){return false;}
    }

    return validate_config();
}

bool Arena::initialize() {
    if(!validate_config()){return false;}

    initialize_board();

    if(!place_obstacles()){return false;}
    if(!load_robots()){return false;}
    if(!place_robots()){return false;}

    return true;
}

void Arena::run() {
    while(!has_winner() && _current_round <= _max_rounds)
    {
        run_round();
        _current_round++;
    }
}

void Arena::run_round() {
    print_round_header();
    print_board();

    for (std::size_t i=0; i < _robots.size(); i++){
        if(has_winner()){
            return;
        }

        process_robot_turn(i);
        print_robot_status(i);
    }
}

void Arena::process_robot_turn(std::size_t robot_index){
    RobotBase* robot = _robots[robot_index];
    if (!is_robot_alive(robot_index)) {return;}

    int radar_direction = 0;
    robot->get_radar_direction(radar_direction);

    std::vector<RadarObj> results = perform_radar_scan(robot_index, radar_direction);
    robot->process_radar_results(results);

    if(!process_shot(robot_index)){process_movement(robot_index);}
}

bool Arena::has_winner() const {
    return living_robot_count() == 1;
}

bool Arena::is_robot_alive(std::size_t robot_index) const {
    return _robots[robot_index]->get_health() > 0;
}

int Arena::winner_index() const {
    if(!has_winner()){return -1;}
    for (std::size_t i=0; i < _robots.size(); i++){
        if (is_robot_alive(i)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

int Arena::living_robot_count() const {
    int count = 0;
        for (std::size_t i=0; i < _robots.size(); i++){
            if (is_robot_alive(i)) {
                count ++;
            }
    }
    return count;
}

void Arena::print_round_header() const {
    std::cout << "========== starting round " << _current_round << " ==========" << std::endl;
}

void Arena::print_board() const {
    for (int row = 0; row < _arena_height; row++){
        for (int column = 0; column < _arena_width; column++){
            std::cout << board_symbol_at(row, column) << ' ';
        }
        std::cout << '\n';
    }
}

void Arena::print_robot_status(std::size_t robot_index) const {
    if (robot_index >= _robots.size()){return;}

    char robot_state = 'R';
    if (!is_robot_alive(robot_index))
    {
        robot_state = 'X';
    }

    std::cout << robot_state << _robots[robot_index]->m_character << ' '
              << _robots[robot_index]->print_stats() << '\n';
}

char Arena::board_symbol_at(int row, int col) const {
    int index = robot_at(row, col);
    if (index != -1){
        if (is_robot_alive(index)){
            return _robots[index]->m_character;
        }
        else{return 'X';}
    }

    return _board[row][col];
}

void Arena::initialize_board(){
    _board = std::vector<std::vector<char>>(
        _arena_height,
        std::vector<char>(_arena_width, '.')
    );
}

bool Arena::is_cell_empty(int row, int col) const{
    if (!is_in_bounds(row, col)){return false;}
    if (robot_at(row, col) != -1){return false;}
    if (_board[row][col] != '.'){return false;}
    return true;
}

int Arena::robot_at(int row, int col) const{
    for (std::size_t i = 0; i < _robots.size(); i++){
        int robot_row = 0;
        int robot_col = 0;
        _robots[i]->get_current_location(robot_row, robot_col);

        if (robot_row == row && robot_col == col){
            return static_cast<int>(i);
        }
    }

    return -1;
}

std::pair<int, int> Arena::random_empty_cell() const{
    while (true){
        int row = rand() % _arena_height;
        int col = rand() % _arena_width;
        if (is_cell_empty(row,col)){
            return {row,col};
        }
    }
}

bool Arena::place_single_obstacle(char obstacle_type){
    if(obstacle_type != 'M' && obstacle_type != 'P' && obstacle_type != 'F'){return false;}
    std::pair<int,int> cell = random_empty_cell();
    _board[cell.first][cell.second] = obstacle_type;
    return true;
}

bool Arena::place_obstacles(){
    for (int i = 0; i < _num_flamethrowers; i++){
        if (!place_single_obstacle('F')){return false;}
    }
    for (int i = 0; i < _num_mounds; i++){
        if (!place_single_obstacle('M')){return false;}
    }
    for (int i = 0; i < _num_pits; i++){
        if (!place_single_obstacle('P')){return false;}
    }
    return true;
}

bool Arena::validate_config() const {
    if(_arena_height <= 0){return false;}
    if(_arena_width <= 0){return false;}
    if(_max_rounds <= 0){return false;}
    if(_num_mounds < 0){return false;}
    if(_num_pits < 0){return false;}
    if(_num_flamethrowers < 0){return false;}
    if(_num_mounds + _num_pits + _num_flamethrowers >= _arena_height * _arena_width){return false;}
    return true;
}

bool Arena::parse_config_line(const std::string& line){
    std::size_t colon_pos = line.find(':');
    if (colon_pos==std::string::npos){return false;}

    std::string key = line.substr(0,colon_pos);
    std::string value = line.substr(colon_pos + 1);
    std::istringstream value_stream(value);
    
    if (key == "Arena_Size"){
        value_stream >> _arena_height >> _arena_width;
        return !value_stream.fail();
    }
    else if (key == "Max_Rounds"){
        value_stream >> _max_rounds;
        return !value_stream.fail();
    }
    else if (key == "Flamethrowers")
    {
        value_stream >> _num_flamethrowers;
        return !value_stream.fail();
    }
    else if (key == "Pits")
    {
        value_stream >> _num_pits;
        return !value_stream.fail();
    }
    else if (key == "Mounds")
    {
        value_stream >> _num_mounds;
        return !value_stream.fail();
    }
    else if (key == "Sleep_interval")
    {
        double ignored_sleep = 0.0;
        value_stream >> ignored_sleep;
        return !value_stream.fail();
    }
    else if (key == "Game_State_Live")
    {
        std::string ignored_live;
        value_stream >> ignored_live;
        return ignored_live == "true" || ignored_live == "false";
    }

    return false;
}

bool Arena::load_robots(){
    std::vector<std::filesystem::path> robot_sources = find_robot_sources();

    if (robot_sources.empty()){return false;}

    for(std::size_t i=0; i< robot_sources.size(); i++){
        std::filesystem::path source_path = robot_sources[i];
        std::filesystem::path shared_lib_path = "./lib" + source_path.stem().string() + ".so";

        if (!compile_robot(source_path, shared_lib_path)){continue;}

        void* handle = nullptr;
        RobotBase* robot = create_robot_instance(shared_lib_path, handle);
        if (robot == nullptr){continue;}

        robot->m_character = assign_robot_symbol(i);
        robot->set_boundaries(_arena_height, _arena_width);
        _robots.push_back(robot);
        _robot_handles.push_back(handle);
    }

    return !_robots.empty();
}

std::vector<std::filesystem::path> Arena::find_robot_sources() const{
    std::vector<std::filesystem::path> robot_sources;
    std::filesystem::path robots_dir = "robots";

    if(!std::filesystem::exists(robots_dir)){return robot_sources;}

    for (const auto& entry : std::filesystem::directory_iterator(robots_dir)){
        if(!entry.is_regular_file()){continue;}

        std::filesystem::path path = entry.path();
        std::string filename = path.filename().string();

        if(filename.rfind("Robot_", 0) == 0 && path.extension() == ".cpp"){robot_sources.push_back(path);}
    }

    std::sort(robot_sources.begin(), robot_sources.end());
    return robot_sources;
}

bool Arena::compile_robot(const std::filesystem::path& source_path, const std::filesystem::path& shared_lib_path) const {
    std::string compile_cmd =
        "g++ -shared -fPIC -o " + shared_lib_path.string() + " " +
        source_path.string() + " RobotBase.o -I. -std=c++20";
    
    int result = std::system(compile_cmd.c_str());
    return result == 0;
}

RobotBase* Arena::create_robot_instance(const std::filesystem::path& shared_lib_path, void*& handle) const {
    handle = dlopen(shared_lib_path.c_str(), RTLD_LAZY);
    if(handle == nullptr){
        std::cerr << "dlopen failed for " << shared_lib_path << ": " << dlerror() << '\n';
        return nullptr;
    }

    RobotFactory create_robot = reinterpret_cast<RobotFactory>(dlsym(handle, "create_robot"));

    if(create_robot == nullptr) {
        dlclose(handle);
        handle = nullptr;
        return nullptr;
    }

    RobotBase* robot = create_robot();
    if(robot == nullptr) {
        dlclose(handle);
        handle = nullptr;
        return nullptr;
    }

    return robot;
}

char Arena::assign_robot_symbol(std::size_t robot_index) const{
    std::string symbols = "!@#$%^&*";
    if(robot_index < symbols.size()){return symbols[robot_index];}

    return '?';
}

bool Arena::place_robots(){
    for (std::size_t i=0; i < _robots.size(); i++){
        if(!place_single_robot(i)){return false;}
    }
    return true;
}

bool Arena::place_single_robot(std::size_t robot_index){
    if(robot_index >= _robots.size()){return false;}

    std::pair<int,int> cell = random_empty_cell();
    int row = cell.first;
    int col = cell.second;

    _robots[robot_index]->move_to(row,col);

    return true;
}

void Arena::cleanup_robots(){
    for(std::size_t i=0; i < _robots.size(); i++){delete _robots[i];}
    _robots.clear();
    
    for(std::size_t i=0; i < _robot_handles.size(); i++){
        if(_robot_handles[i] != nullptr){dlclose(_robot_handles[i]);}
    }
    _robot_handles.clear();
}

bool Arena::is_in_bounds(int row, int col) const{
    if(row >= 0 && row < _arena_height && col >= 0 && col < _arena_width){return true;}
    return false;
}

std::vector<RadarObj> Arena::perform_radar_scan(std::size_t robot_index, int direction) const{
    if (direction == 0)
    {
        return scan_adjacent(robot_index);
    }

    if (direction < 0 || direction > 8)
    {
        return {};
    }

    return scan_diretional_ray(robot_index, direction);
}

std::vector<RadarObj> Arena::scan_adjacent(std::size_t robot_index) const{
    std::vector<RadarObj> results;
    if (robot_index >= _robots.size()){return results;}

    int current_row = 0;
    int current_col = 0;
    _robots[robot_index]->get_current_location(current_row, current_col);

    for (int direction = 1; direction <= 8; direction++)
    {
        int row = current_row + directions[direction].first;
        int col = current_col + directions[direction].second;
        add_radar_cell(results, robot_index, row, col);
    }

    return results;
}

std::vector<RadarObj> Arena::scan_diretional_ray(std::size_t robot_index, int direction) const{
    std::vector<RadarObj> results;
    if (robot_index >= _robots.size() || direction < 1 || direction > 8){return results;}

    int current_row = 0;
    int current_col = 0;
    _robots[robot_index]->get_current_location(current_row, current_col);

    int delta_row = directions[direction].first;
    int delta_col = directions[direction].second;
    int perp_row = -delta_col;
    int perp_col = delta_row;

    for (int step = 1; ; step++)
    {
        int center_row = current_row + delta_row * step;
        int center_col = current_col + delta_col * step;

        if (!is_in_bounds(center_row, center_col))
        {
            break;
        }

        for (int offset = -1; offset <= 1; offset++)
        {
            int row = center_row + perp_row * offset;
            int col = center_col + perp_col * offset;
            add_radar_cell(results, robot_index, row, col);
        }
    }

    return results;
}

void Arena::add_radar_cell(std::vector<RadarObj>& results, std::size_t scanning_robot_index, int row, int col) const{
    if (!is_in_bounds(row, col)){return;}

    int scanning_row = 0;
    int scanning_col = 0;
    _robots[scanning_robot_index]->get_current_location(scanning_row, scanning_col);
    if (row == scanning_row && col == scanning_col){return;}

    int robot_index = robot_at(row, col);
    if (robot_index != -1)
    {
        results.push_back(RadarObj(is_robot_alive(robot_index) ? 'R' : 'X', row, col));
        return;
    }

    char cell = _board[row][col];
    if (cell != '.')
    {
        results.push_back(RadarObj(cell, row, col));
    }
}

bool Arena::process_shot(std::size_t robot_index){
    if (robot_index >= _robots.size()){return false;}

    int shot_row = 0;
    int shot_col = 0;
    if (!_robots[robot_index]->get_shot_location(shot_row, shot_col))
    {
        return false;
    }

    int current_row = 0;
    int current_col = 0;
    _robots[robot_index]->get_current_location(current_row, current_col);

    WeaponType weapon = _robots[robot_index]->get_weapon();

    auto apply_to_cell = [this, robot_index](int row, int col, int damage)
    {
        if (!is_in_bounds(row, col)){return;}
        int target_index = robot_at(row, col);
        if (target_index != -1 && target_index != static_cast<int>(robot_index))
        {
            damage_robot(static_cast<std::size_t>(target_index), damage);
        }
    };

    if (weapon == grenade)
    {
        if (_robots[robot_index]->get_grenades() <= 0)
        {
            return false;
        }

        _robots[robot_index]->decrement_grenades();
        int damage = roll_weapon_damage(weapon);
        for (int row = shot_row - 1; row <= shot_row + 1; row++)
        {
            for (int col = shot_col - 1; col <= shot_col + 1; col++)
            {
                apply_to_cell(row, col, damage);
            }
        }
        return true;
    }

    int delta_row = 0;
    if (shot_row > current_row){delta_row = 1;}
    else if (shot_row < current_row){delta_row = -1;}

    int delta_col = 0;
    if (shot_col > current_col){delta_col = 1;}
    else if (shot_col < current_col){delta_col = -1;}

    if (delta_row == 0 && delta_col == 0)
    {
        return true;
    }

    if (weapon == railgun)
    {
        int damage = roll_weapon_damage(weapon);
        int row = current_row + delta_row;
        int col = current_col + delta_col;
        while (is_in_bounds(row, col))
        {
            apply_to_cell(row, col, damage);
            row += delta_row;
            col += delta_col;
        }
        return true;
    }

    if (weapon == flamethrower)
    {
        int perp_row = -delta_col;
        int perp_col = delta_row;
        int damage = roll_weapon_damage(weapon);
        for (int step = 1; step <= 4; step++)
        {
            int center_row = current_row + delta_row * step;
            int center_col = current_col + delta_col * step;
            for (int offset = -1; offset <= 1; offset++)
            {
                int row = center_row + perp_row * offset;
                int col = center_col + perp_col * offset;
                apply_to_cell(row, col, damage);
            }
        }
        return true;
    }

    int distance_row = std::abs(shot_row - current_row);
    int distance_col = std::abs(shot_col - current_col);
    if (weapon == hammer && std::max(distance_row, distance_col) > 1)
    {
        return false;
    }

    apply_to_cell(shot_row, shot_col, roll_weapon_damage(weapon));
    return true;
}

int Arena::roll_weapon_damage(WeaponType weapon) const{
    switch (weapon)
    {
        case flamethrower:
            return 30 + (std::rand() % 21);
        case railgun:
            return 10 + (std::rand() % 11);
        case grenade:
            return 10 + (std::rand() % 31);
        case hammer:
            return 50 + (std::rand() % 11);
    }

    return 0;
}

void Arena::damage_robot(std::size_t robot_index, int damage){
    if (robot_index >= _robots.size() || damage <= 0){return;}

    int armor = _robots[robot_index]->get_armor();
    int reduced_damage = static_cast<int>(damage * (1.0 - (armor * 0.1)));
    if (reduced_damage < 0){reduced_damage = 0;}

    _robots[robot_index]->take_damage(reduced_damage);
    _robots[robot_index]->reduce_armor(1);
}

void Arena::process_movement(std::size_t robot_index){
    if (robot_index >= _robots.size()){return;}

    int direction = 0;
    int distance = 0;
    _robots[robot_index]->get_move_direction(direction, distance);

    if (direction < 1 || direction > 8 || distance <= 0)
    {
        return;
    }

    int max_distance = _robots[robot_index]->get_move_speed();
    if (distance > max_distance)
    {
        distance = max_distance;
    }

    for (int step = 0; step < distance; step++)
    {
        if (!move_robot_one_step(robot_index, direction))
        {
            return;
        }
    }
}

bool Arena::move_robot_one_step(std::size_t robot_index, int direction){
    if (robot_index >= _robots.size() || direction < 1 || direction > 8){return false;}

    int current_row = 0;
    int current_col = 0;
    _robots[robot_index]->get_current_location(current_row, current_col);

    int next_row = current_row + directions[direction].first;
    int next_col = current_col + directions[direction].second;

    if (!is_in_bounds(next_row, next_col)){return false;}
    if (robot_at(next_row, next_col) != -1){return false;}

    char cell = _board[next_row][next_col];
    if (cell == 'M')
    {
        return false;
    }

    _robots[robot_index]->move_to(next_row, next_col);

    if (cell == 'P')
    {
        _robots[robot_index]->disable_movement();
        return false;
    }

    if (cell == 'F')
    {
        damage_robot(robot_index, roll_weapon_damage(flamethrower));
        if (!is_robot_alive(robot_index))
        {
            _board[next_row][next_col] = '.';
            return false;
        }
    }

    return true;
}

Arena::~Arena(){cleanup_robots();}
