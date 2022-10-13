#include "tournament.h"
#include "games.h"
#include "stdlib.h"
#include "assert.h"
#include "string.h"
#include "stdio.h"
#include "stdbool.h"
#define TOUR_NOT_OVER (-1)
#define DUMMY_LIMB_ID (-1)
#define EMPTY (-1)
#define EQUAL 0



struct tour_data
{
    char* tournament_location;
    int max_game_player;
    MapHead games;
    Map Players;
    int tournament_winner;
    bool tour_ended;
};
/* free tha tournament data */
static void tournamentFreeData(MapDataElement tournament_data);

/* copy the tournament data */
static MapDataElement tournamentDataCopy(MapDataElement tournament_data);


Map tournamentCreateNew()
{
    Map new_tournament_map = mapCreate(tournamentDataCopy, copyIntKey, tournamentFreeData,
                                       destroyIntKey, intKeyCompare);
    if(new_tournament_map == NULL){
        return NULL;
    }
    return new_tournament_map;
}
TournamentResult tournamentAdd(Map tournament, int tournament_id,
                               const char* location, int max_games)
{
    assert(tournament != NULL && location != NULL);
    TourData tour_data = malloc(sizeof(*tour_data));
    if(tour_data == NULL){
        return TOUR_MEMORY_PROBLEM;
    }
    tour_data->tournament_location = malloc(strlen(location) + 1);
    if(tour_data->tournament_location == NULL)
    {
        free(tour_data);
        return TOUR_MEMORY_PROBLEM;
    }
    strcpy(tour_data->tournament_location, location);
    tour_data->tournament_winner = TOUR_NOT_OVER;
    tour_data->tour_ended = false;
    tour_data->max_game_player = max_games;
    tour_data->games = gameCreate();
    tour_data->Players = playersCreateMap();
    if(tour_data->games == NULL || tour_data->Players == NULL ||
       mapPut(tournament, &tournament_id, tour_data) == MAP_OUT_OF_MEMORY){
        tournamentFreeData(tour_data);
        return TOUR_MEMORY_PROBLEM;
    }
    tournamentFreeData(tour_data);
    return TOUR_SUCCESS;
}

TournamentResult tournamentAddGame(Map tournament, int tournament_id, int id1, int id2,
                                   Winner winner, int game_time)
{
    assert(id1 > 0 && id2 > 0 && tournament != NULL);
    if(mapContains(tournament, &tournament_id) == false){
        return TOUR_NOT_EXIST;
    }
    TourData tour_data = mapGet(tournament, &tournament_id);
    if(tour_data->tour_ended == true){
        return TOUR_ENDED;
    }
    if(gameExist(tour_data->games, id1, id2)){
        return TOUR_EXIST_GAME;
    }
    if(game_time < 0){
        return TOUR_NEGATIVE_TIME;
    }
    if (playerGetNumOfGames(tour_data->Players, &id1) >= tour_data->max_game_player ||
        playerGetNumOfGames(tour_data->Players, &id2) >= tour_data->max_game_player){
        return TOUR_EXCEEDED_GAMES;
    }
    if(gameAdd(tour_data->games, tour_data->Players, id1, id2, winner, game_time) == GAME_OUT_OF_MEMORY) {
        return TOUR_MEMORY_PROBLEM;
    }
    return TOUR_SUCCESS;
}
TournamentResult tournamentStatistic(char* path_file, Map tournament)
{
    FILE* file = fopen(path_file,"w");
    if(file == NULL){
        return TOUR_MEMORY_PROBLEM;
    }
    MAP_FOREACH(int*, iter, tournament)
    {
        TourData tour_data = mapGet(tournament, iter);
        if(tour_data->tour_ended == true) {
            if (fprintf(file, "%d\n", tour_data->tournament_winner) < 0) {
                return TOUR_MEMORY_PROBLEM;
            }
            if (fprintf(file, "%d\n", gameGetInfo(tour_data->games, GAME_LONGEST)) < 0) {
                return TOUR_MEMORY_PROBLEM;
            }
            if (fprintf(file, "%.2lf\n", gameAvgTime(tour_data->games)) < 0) {
                return TOUR_MEMORY_PROBLEM;
            }
            if (fprintf(file, "%s\n", tour_data->tournament_location) < 0) {
                return TOUR_MEMORY_PROBLEM;
            }
            if (fprintf(file, "%d\n", gameGetInfo(tour_data->games, GAME_NUM_GAMES)) < 0) {
                return TOUR_MEMORY_PROBLEM;
            }
            if (fprintf(file, "%d\n", gameGetInfo(tour_data->games, GAME_NUM_PLAYERS)) < 0) {
                return TOUR_MEMORY_PROBLEM;
            }
        }
        free(iter);
    }
    fclose(file);
    return TOUR_SUCCESS;
}
void tournamentDestroy(Map tournament, Map external_player, int id)
{
    assert(tournament!= NULL && external_player != NULL);
    TourData data = mapGet(tournament, &id);
    playerRemoveData(external_player, data->Players);
    mapRemove(tournament, &id);
}

void tournamentRemovePlayer(Map tournament, Map external_player, int player_id)
{
    assert(tournament != NULL && external_player != NULL);
    MAP_FOREACH(int*, iter, tournament) {
        TourData tour_data = mapGet(tournament, iter);
        if(mapContains(tour_data->Players, &player_id)) {
            if(tour_data->tour_ended == false) {
                gameUpdateQuitPlayer(tour_data->games, tour_data->Players, external_player, player_id);
                mapRemove(tour_data->Players, &player_id);
            }
            else {
                mapRemove(tour_data->Players, &player_id);
            }
        }
        free(iter);
    }
}
TournamentResult tournamentEnd(Map tournament, int tour_id)
{
    assert(tournament != NULL);
    TourData data = mapGet(tournament, &tour_id);
    assert(data != NULL);
    if(data->tour_ended == true){
        return TOUR_ENDED;
    }
    if(data->games == NULL || gameGetInfo(data->games, GAME_NUM_GAMES) == 0){
        return TOUR_N0_GAMES;
    }
    data->tour_ended = true;
    data->tournament_winner = playerGetTournamentWinnerID(data->Players);
    return TOUR_SUCCESS;
}

double tournamentCalculateTotalTime(Map tournament, int player_id)
{
    if(tournament == NULL) {
        return 0;
    }
    double average_time = 0;
    MAP_FOREACH(int*, iter, tournament)
    {
        TourData data = mapGet(tournament, iter);
        if(mapContains(data->Players, &player_id)) {
            average_time += (gamePlayedTime(data->games, player_id));
        }
        destroyIntKey(iter);
    }
    return average_time;

}
bool tournamentExistEnded(Map tournament)
{
    MAP_FOREACH(int*, iter, tournament)
    {
        TourData data = mapGet(tournament, iter);
        if(data->tour_ended == true) {
            destroyIntKey(iter);
            return true;
        }
        destroyIntKey(iter);
    }
    return false;
}

static void tournamentFreeData(MapDataElement tournament_data)
{
    if (tournament_data == NULL) {
        return;
    }
    TourData data = tournament_data;
    if(data->tournament_location != NULL)
    {
        free(data->tournament_location);
    }
    if(data->games != NULL)
    {
        gameDestroy(data->games);
    }
    if(data->Players != NULL)
    {
        mapDestroy(data->Players);
    }
    free(data);
}
static MapDataElement tournamentDataCopy(MapDataElement tournament_data)
{
    TourData old_data = tournament_data;
    TourData new_data = malloc(sizeof(*new_data));
    if(new_data == NULL){
        return NULL;
    }
    new_data->tournament_location = malloc(strlen(old_data->tournament_location) + 1);
    if(new_data->tournament_location == NULL){
        free(new_data);
        return NULL;
    }
    strcpy(new_data->tournament_location, old_data->tournament_location);
    new_data->games = gameCopy(old_data->games);
    if(new_data->games == NULL){
        tournamentFreeData(new_data);
        return NULL;
    }
    new_data->Players = mapCopy(old_data->Players);
    if(new_data->Players == NULL){
        tournamentFreeData(new_data);
        return NULL;
    }
    new_data->tournament_winner = old_data->tournament_winner;
    new_data->max_game_player = old_data->max_game_player;
    new_data->tour_ended = old_data->tour_ended;
    return new_data;
}


