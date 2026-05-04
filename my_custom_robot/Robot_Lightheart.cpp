#include "RobotBase.h"
#include <vector>
#include <cmath>
#include <algorithm>

class Lightheart_Robot : public RobotBase
{
private:
    int m_target_row = -1;
    int m_target_col = -1;
    int m_radar_dir = 1; // rotates 1-8 when no target is known
    int m_damage_dealt = 0;

    bool has_target() const { return m_target_row != -1; }

    // Returns the direction index (1-8) toward (dr, dc) delta
    int direction_toward(int dr, int dc) const
    {
        int row_sign = (dr > 0) ? 1 : (dr < 0) ? -1 : 0;
        int col_sign = (dc > 0) ? 1 : (dc < 0) ? -1 : 0;
        for (int d = 1; d <= 8; d++)
        {
            if (directions[d].first == row_sign && directions[d].second == col_sign)
                return d;
        }
        return 0;
    }

public:
    Lightheart_Robot() : RobotBase(3, 4, hammer) {}

    virtual void get_radar_direction(int& radar_direction) override
    {
        if (has_target())
        {
            int cur_row, cur_col;
            get_current_location(cur_row, cur_col);
            int d = direction_toward(m_target_row - cur_row, m_target_col - cur_col);
            if (d != 0)
            {
                radar_direction = d;
                return;
            }
        }
        // No target — rotate through all directions to search
        radar_direction = m_radar_dir;
        m_radar_dir = (m_radar_dir % 8) + 1;
    }

    virtual void process_radar_results(const std::vector<RadarObj>& radar_results) override
    {
        for (const auto& obj : radar_results)
        {
            if (obj.m_type == 'R')
            {
                // Reset damage counter when switching to a different target
                if (obj.m_row != m_target_row || obj.m_col != m_target_col)
                    m_damage_dealt = 0;
                m_target_row = obj.m_row;
                m_target_col = obj.m_col;
                return;
            }
        }
    }

    // Always attempt to hammer the target — the arena only accepts it when adjacent,
    // and falls back to movement otherwise.
    virtual bool get_shot_location(int& shot_row, int& shot_col) override
    {
        if (!has_target()) return false;

        int cur_row, cur_col;
        get_current_location(cur_row, cur_col);
        int dist = std::max(std::abs(m_target_row - cur_row), std::abs(m_target_col - cur_col));

        if (dist <= 1)
        {
            m_damage_dealt += 50; // minimum hammer damage per swing
            if (m_damage_dealt >= 100)
            {
                m_target_row = -1;
                m_target_col = -1;
                m_damage_dealt = 0;
            }
        }

        shot_row = m_target_row == -1 ? cur_row : m_target_row;
        shot_col = m_target_col == -1 ? cur_col : m_target_col;
        return m_target_row != -1;
    }

    virtual void get_move_direction(int& move_direction, int& move_distance) override
    {
        if (!has_target())
        {
            move_direction = 0;
            move_distance = 0;
            return;
        }

        int cur_row, cur_col;
        get_current_location(cur_row, cur_col);

        int dr = m_target_row - cur_row;
        int dc = m_target_col - cur_col;
        int dist = std::max(std::abs(dr), std::abs(dc));

        if (dist == 0)
        {
            // Reached last known position but enemy is gone — resume searching
            m_target_row = -1;
            m_target_col = -1;
            move_direction = 0;
            move_distance = 0;
            return;
        }

        move_direction = direction_toward(dr, dc);
        move_distance = std::min(get_move_speed(), dist);
    }
};

extern "C" RobotBase* create_robot()
{
    return new Lightheart_Robot();
}
