#include "rule.h"
#include "option.h"

void normalize(std::vector<double>& p);
int roulette_wheel(std::vector<double>& p, Random* rnd);


void init_cell(Cell* cell);
void init_grain_for_green(Cell* cell);
void init_soil_conservation(Cell* cell);

void init_cell(Cell* cell)
{
    int land_use = g_land_use_map.atxy(cell->x, cell->y);
    cell->value = land_use;
    cell->type = 1;
    cell->transP.assign(g_max, 1.0 / g_max);

    if (land_use == g_nodata) {
        cell->type = 0;
        return;
    }

    switch (land_use) {
        case 1:
            init_grain_for_green(cell);
            break;
        case 3:
        case 4:
            init_soil_conservation(cell);
            break;
        case 5:
        case 7:
        case 8:
        case 9:
        case 10:
            cell->type = 0;
            break;
     }
}

void init_grain_for_green(Cell* cell)
{
    double slope = g_slope_map.atxy(cell->x, cell->y);
    if (slope >= 25.0) {
        cell->value = 3;
        cell->type = 0;
    }
}

void init_soil_conservation(Cell* cell)
{
    std::vector<int> neighbors = g_land_use_map.neighbors(cell->x, cell->y, 2);
    for (int i=0; i < neighbors.size(); ++i) {
        if (neighbors.at(i) == 9) {
            cell->type = 0;
        }
    }
}


int transition(Cell* cell);
bool rule_cell_type(Cell* cell);
bool rule_edge_cell(Cell* cell);
bool rule_quantity(Cell* cell, int value, int max);
bool rule_neighbors_has(Cell* cell, int radius, int value);
bool rule_soil_condition(Cell* cell);
bool rule_farming_radius(Cell* cell, int radius);
bool rule_road_access(Cell* cell, double distance);
bool rule_suitability(Cell* cell, int value, double threshold);

int transition(Cell* cell)
{
    if ( !rule_cell_type(cell) ) return cell->value;
    if ( !rule_edge_cell(cell) ) return cell->value;

    bool is_rule_success = false;
    int new_value = roulette_wheel(cell->transP, g_RND);
    switch (new_value) {
        case 1:
            is_rule_success =
                rule_quantity(cell, 1, g_arable) &&
                rule_suitability(cell, 1, 0.3) &&
                rule_soil_condition(cell) &&
                rule_farming_radius(cell, 40);
            break;
        case 2:
            is_rule_success =
                rule_quantity(cell, 2, g_orchard) &&
                rule_road_access(cell, 500);
            break;
        case 3:
            is_rule_success =
                rule_quantity(cell, 3, g_forest);
            break;
        case 4:
            break;
        case 5:
            break;
        case 6:
            break;
        case 7:
            break;
        case 8:
            break;
        case 9:
            break;
    }

    return is_rule_success ? new_value : cell->value;
}

bool rule_quantity(Cell* cell, int value, int max)
{
    int count = cell->map->counts.at(value);
    return count < max;
}

bool rule_neighbors_has(Cell* cell, int radius, int value)
{
    std::vector<Cell*> neighbors = cell->map->neighbors(cell->x, cell->y, radius);
    for (int i=0; i < neighbors.size(); ++i) {
        if (neighbors.at(i)->value == value) return true;
    }

    return false;
}

bool rule_cell_type(Cell* cell)
{
    return cell->type > 0;
}

bool rule_edge_cell(Cell* cell)
{
    bool is_edge_cell = false;
    std::vector<Cell*> neighbors = cell->map->neighbors(cell->x, cell->y, g_edge_depth);
    for (int i=0; i < neighbors.size(); ++i) {
        if (neighbors.at(i)->value != cell->value) {
            is_edge_cell = true;
            break;
        }
    }

    double p = g_RND->nextDouble();
    if (is_edge_cell) {
        if (p >= g_edge) return true;
    } else {
        if (p >= g_core) return true;
    }

    return false;
}

bool rule_soil_condition(Cell* cell)
{
    double soil_depth = g_soil_depth_map.atxy(cell->x, cell->y);
    double slope = g_slope_map.atxy(cell->x, cell->y);

    return soil_depth >= 0.3 && slope < 25;
}

bool rule_farming_radius(Cell* cell, int radius)
{
    return rule_neighbors_has(cell, radius, 6);
}

bool rule_road_access(Cell* cell, double distance)
{
    double dist = g_road_map.atxy(cell->x, cell->y);

    return dist <= distance;
}

bool rule_suitability(Cell* cell, int value, double threshold)
{
    double suit = 0.0;
    switch (value)
    {
        case 1:
            suit = g_arable_suit_map.atxy(cell->x, cell->y);
            break;
        case 2:
            suit = g_orchard_suit_map.atxy(cell->x, cell->y);
            break;
        case 3:
            suit = g_forest_suit_map.atxy(cell->x, cell->y);
            break;
        case 5:
        case 6:
            suit = g_construction_suit_map.atxy(cell->x, cell->y);
            break;
    }

    return suit >= threshold;

}
