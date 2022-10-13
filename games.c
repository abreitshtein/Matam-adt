#include <stdio.h>
#include "games.h"
#include "stdlib.h"
#include "assert.h"
#define QUIT (-2)
#define DELETE (-1)
#define ADD 1
#define DRAW (-1)
#define EQUAL 0



struct Map_Head
{
    Map head;
    int longest_game;
    int number_games;
    int number_players;
    double total_time;
};
struct GameData_t
{
    int first_id;
    int second_id;
    int time;
    int winner;
};
/* copy the game data. pointer if success, NULL if failed. */
static MapDataElement copyGameData(MapDataElement game_data);
/* update player both list */
static void updatePlayerBothData(Map internal_player, Map external_player, PlayerFlag game_result,
                                 int id, int factor);
/* update the data and score of a quit player. */
static void checkAndUpdateScore(GameData data, int check_id, Map internal_player,
                                Map external_player, int id_quit);
/* convert the winner enum to the winner id */
static int convertWinnerToInt(int first_id, int second_id, Winner winner);
/* update the game data */
static void updateGameData(GameData data, int id1, int id2, int winner, int time);
/* allocate a new data and key. return NULL if failed. */
static GameResult allocateNew(GameData* data, int** key);
/* free the game data and key */
static void freeDataAndKey(int* key, GameData data);
/* update a data about a new game added */
static void updateNewGame(MapHead game, int time, bool first, bool second);
/* compare between 2 pairs of numbers */
static bool compareNumber(int first, int second, int first2, int second2);

MapHead gameCreate()
{
    MapHead new = malloc(sizeof(*new));
    if(new == NULL) {
        return NULL;
    }
    new->head = mapCreate(copyGameData, copyIntKey, destroyNode, destroyIntKey, intKeyCompare);
    if(new->head == NULL){
        free(new);
        return NULL;
    }
    new->longest_game = 0;
    new->number_games = 0;
    new->number_players = 0;
    new->total_time = 0;
    return new;
}
MapHead gameCopy(MapHead game)
{
    MapHead new = malloc(sizeof(*new));
    if(new == NULL){
        return NULL;
    }
    new->head = mapCopy(game->head);
    if(new->head == NULL){
        free(new);
        return NULL;
    }
    new->total_time = game->total_time;
    new->number_games = game->number_games;
    new->longest_game = game->longest_game;
    new->number_players = game->number_players;
    return new;
}
void gameDestroy(MapHead game)
{
    if(game == NULL){
        return;
    }
    mapDestroy(game->head);
    free(game);
}
GameResult gameAdd(MapHead game, Map players, int first_id, int second_id, Winner winner_id, int play_time)
{
    assert(first_id > 0 && second_id > 0 && play_time >= 0 && game != NULL && players != NULL);
    int* new_id = NULL;
    GameData new_data = NULL;
    if(allocateNew(&new_data, &new_id)== GAME_OUT_OF_MEMORY){
        return GAME_OUT_OF_MEMORY;
    }
    *new_id = game->number_games;
    int winner = convertWinnerToInt(first_id, second_id, winner_id);
    updateGameData(new_data, first_id, second_id, winner, play_time);
    if(mapPut(game->head, new_id, new_data) == MAP_OUT_OF_MEMORY)
    {
        freeDataAndKey(new_id, new_data);
        return GAME_OUT_OF_MEMORY;
    }
    bool first_alloc = false, second_alloc = false;
    if(playersAdd(players, first_id, second_id, &first_alloc, &second_alloc) == PLAYER_MEMORY_FAILED)
    {
        mapRemove(game->head, &new_id);
        freeDataAndKey(new_id, new_data);
        return GAME_OUT_OF_MEMORY;
    }
    updateExternalPlayer(players, winner_id, first_id, second_id);
    updateNewGame(game, play_time, first_alloc, second_alloc);
    freeDataAndKey(new_id, new_data);
    return GAME_SUCCESS;
}
void gameUpdateQuitPlayer(MapHead game, Map internal_player,
                          Map external_player, int id_quit_player)
{
    assert(game != NULL && internal_player != NULL && external_player != NULL);
    MAP_FOREACH(int*, iter, game->head)
    {
        GameData current_data = mapGet(game->head, iter);
        if(current_data->first_id == id_quit_player ) {
            current_data->first_id = QUIT;
            checkAndUpdateScore(current_data, current_data->second_id, internal_player,
                                external_player, id_quit_player);
        }
        if(current_data->second_id == id_quit_player ) {
            current_data->second_id = QUIT;
            checkAndUpdateScore(current_data, current_data->first_id, internal_player,
                                external_player, id_quit_player);
        }
        destroyIntKey(iter);
    }
}
bool gameExist(MapHead game, int first_id, int second_id) {
    if (game == NULL) {
        return false;
    }
    MAP_FOREACH(int*, iter, game->head)
    {
        GameData current_data = mapGet(game->head, iter);
        if(compareNumber(current_data->first_id, current_data->second_id, first_id, second_id)){
            destroyIntKey(iter);
            return true;
        }
        destroyIntKey(iter);
    }
    return false;
}
double gamePlayedTime(MapHead game, int id)
{
    if(game == NULL){
        return 0;
    }
    double time = 0;
    MAP_FOREACH(int*, iter, game->head)
    {
        GameData current_data = mapGet(game->head, iter);
        if(current_data->first_id == id || current_data -> second_id == id){
            time += current_data->time;
        }
        destroyIntKey(iter);
    }
    return time;
}
double gameAvgTime(MapHead game)
{
    if(game->number_games == 0){
        return 0;
    }
    return ((game->total_time)/(game->number_games));
}
int gameGetInfo(MapHead game, GameInformation info)
{
    assert(game != NULL);
    if(game == NULL){
        return GAME_NULL_ARGUMENT;
    }
    if(info == GAME_NUM_PLAYERS){
        return game->number_players;
    }
    if(info == GAME_NUM_GAMES){
        return game->number_games;
    }
    return game->longest_game;
}
static MapDataElement copyGameData(MapDataElement game_data)
{
    if(game_data == NULL){
        return NULL;
    }
    GameData new_data = malloc(sizeof(*new_data));
    if(new_data == NULL){
        return NULL;
    }
    GameData current_data = (GameData)game_data;
    new_data->first_id = current_data->first_id;
    new_data->second_id = current_data->second_id;
    new_data->winner = current_data->winner;
    new_data->time = current_data->time;
    updateGameData(new_data, current_data->first_id,
                   current_data->second_id, current_data->winner, current_data->time);
    return new_data;
}
static void updateGameData(GameData data, int id1, int id2, int winner, int time)
{
    assert(data != NULL);
    data->first_id = id1;
    data->second_id = id2;
    data->winner = winner;
    data->time = time;
}
static void freeDataAndKey(int* key, GameData data)
{
    assert(key != NULL && data != NULL);
    destroyIntKey(key);
    destroyNode(data);
}
static void updateNewGame(MapHead game, int time, bool first, bool second)
{
    if(game->longest_game < time){
        game->longest_game = time;
    }
    game->number_games++;
    game->total_time += time;
    if(first == true){
        game->number_players++;
    }
    if(second == true){
        game->number_players++;
    }
}
static GameResult allocateNew(GameData* data, int** key) {

    *key = malloc(sizeof(int));
    if (*key == NULL) {
        return GAME_OUT_OF_MEMORY;
    }
    *data = malloc(sizeof(**data));
    if (*data == NULL) {
        destroyIntKey(key);
        return GAME_OUT_OF_MEMORY;
    }
    return GAME_SUCCESS;
}
static void checkAndUpdateScore(GameData data, int check_id, Map internal_player,
                                Map external_player, int id_quit_player)
{
    assert(data != NULL && internal_player != NULL && external_player != NULL);
    if(check_id != QUIT)
    {
        if(data->winner == check_id){
            return;
        }
        if(data->winner == id_quit_player){
            updatePlayerBothData(internal_player, external_player, PLAYER_LOSS, check_id, DELETE);
        }
        if(data->winner == DRAW){
            updatePlayerBothData(internal_player, external_player, PLAYER_DRAWS, check_id, DELETE);
        }
        updatePlayerBothData(internal_player, external_player, PLAYER_WIN, check_id, ADD);
        data->winner = check_id;
        return;
    }
    data->winner = QUIT;
}

static void updatePlayerBothData(Map internal_player, Map external_player, PlayerFlag game_result,
                                 int id, int factor)
{
    playerUpdateData(internal_player, &id, game_result, factor);
    playerUpdateData(external_player, &id, game_result, factor);
}
static bool compareNumber(int first, int second, int first2, int second2) {
    if (first == first2 && second == second2) {
        return true;
    }
    if (first == second2 && second == first2) {
        return true;
    }
    return false;
}
static int convertWinnerToInt(int first_id, int second_id, Winner winner)
{
    if(winner == FIRST_PLAYER)
    {
        return first_id;
    }
    if(winner == SECOND_PLAYER)
    {
        return second_id;
    }
    return DRAW;
}
