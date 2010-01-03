#include "mapitems.hpp"

void layers_init();

MAPITEM_LAYER_TILEMAP *layers_game_layer();
MAPITEM_GROUP *layers_game_group();

int layers_num_groups();
MAPITEM_GROUP *layers_get_group(int index);
MAPITEM_LAYER *layers_get_layer(int index);


