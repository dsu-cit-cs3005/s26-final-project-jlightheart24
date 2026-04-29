#include <string>
#include <vector>
#include <filesystem>
#include "RobotBase.h"

class Arena{
public:
    Arena();
    bool load_config(const std::string& config_path);
    bool initialize();
    void run();
    ~Arena();
private:
    bool parse_config_line(const std::string& line);
    bool validate_config() const;
    void initialize_board();
    bool place_obstacles();
    bool place_single_obstacle(char obstacle_type);

    bool load_robots();
    std::vector<std::filesystem::path> find_robot_sources() const;
    bool compile_robot(
        const std::filesystem::path& source_path, 
        const std::filesystem::path& shared_lib_path
    ) const;
    RobotBase* create_robot_instance(
        const std::filesystem::path& shared_lib_path,
        void*& handle
    ) const;
    bool place_robots();
    bool place_single_robot(std::size_t robot_index);
    char assign_robot_symbol(std::size_t robot_index) const;
    void cleanup_robots();

    void run_round();
    void process_robot_turn(std::size_t robot_index);
    bool has_winner() const;
    int winner_index() const;
    int living_robot_count() const;

    std::vector<RadarObj> perform_radar_scan(std::size_t robot_index, int direction) const;
    std::vector<RadarObj> scan_adjacent(std::size_t robot_index) const;
    std::vector<RadarObj> scan_diretional_ray(std::size_t robot_index, int direction) const;
    void add_radar_cell(std::vector<RadarObj>& results, std::size_t scanning_robot_index, int row, int col) const;

    bool process_shot(std::size_t robot_index);
    int roll_weapon_damage(WeaponType weapon) const;
    void damage_robot(std::size_t robot_index, int direction);

    void process_movement(std::size_t robot_index);
    bool move_robot_one_step(std::size_t robot_index, int direction);

    bool is_in_bounds( int row, int col) const;
    bool is_cell_empty( int row, int col) const;
    bool is_robot_alive(std::size_t robot_index) const;
    std::pair<int, int> random_empty_cell() const;
    int robot_at(int row, int col) const;
    char board_symbol_at(int row, int col) const;

    void print_round_header() const;
    void print_board() const;
    void print_robot_status(std::size_t robot_index) const;
    std::vector<RobotBase*> _robots;
    std::vector<void*> _robot_handles;
    int _max_rounds;
    int _current_round;
    int _arena_height;
    int _arena_width;
    std::vector<std::vector<char>> _board;
    int _num_mounds;
    int _num_pits;
    int _num_flamethrowers;
};
