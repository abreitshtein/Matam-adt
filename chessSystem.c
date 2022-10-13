#include "games.h"
#include "tournament.h"
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdio.h>

#define DUMMY_LIMB_ID (-1)
#define MIN 1
#define NOT_VALID 0
#define MIN_PRINT_SUCCESS 0
#define NO_GAMES 0

struct chess_system_t{
    Map tournaments;
    Map external_players;
};


// Checks whether the tournament_location meets the given requirements

static bool isTourLocationValid(const char* tournament_location);

//Checks whether the parameters for adding a new game meet the given requirements

static ChessResult chessAddGamesResults(ChessSystem chess, int tournament_id ,
                                        int max_games_per_player,
                                        const char* tournament_location);

//converts PlayerNodeResult to ChessResult

static ChessResult PlayerLevelsResult(PlayerNodeResult result);

//converts TournamentResult to ChessResult

static ChessResult convertTourToChess(TournamentResult tour_result);

ChessSystem chessCreate() {
    ChessSystem new_chess_system = malloc(sizeof(*new_chess_system));
    if (new_chess_system == NULL)
    {
        return NULL;
    }
    new_chess_system->tournaments = tournamentCreateNew();
    if(new_chess_system->tournaments == NULL){
        chessDestroy(new_chess_system);
        return NULL;
    }
    new_chess_system->external_players = playersCreateMap();
    if(new_chess_system->external_players == NULL){
        chessDestroy(new_chess_system);
        return NULL;
    }
    return new_chess_system;
}

void chessDestroy(ChessSystem chess){
    if(chess == NULL)
    {
        return;
    }
    mapDestroy(chess->tournaments);
    mapDestroy(chess->external_players);
    free(chess);
}


ChessResult chessAddTournament (ChessSystem chess, int tournament_id, int max_games_per_player,
                                const char* tournament_location){
    if(chess == NULL || tournament_location == NULL){
        return CHESS_NULL_ARGUMENT;
    }
    ChessResult result1 = chessAddGamesResults(chess , tournament_id , max_games_per_player,
                                              tournament_location);
    if(result1 != CHESS_SUCCESS){
        return result1;
    }
    TournamentResult result = tournamentAdd(chess->tournaments,tournament_id,
                                            tournament_location,max_games_per_player);
    if(result == TOUR_MEMORY_PROBLEM)
    {
        return CHESS_OUT_OF_MEMORY;
    }
    return CHESS_SUCCESS;
}


ChessResult chessAddGame (ChessSystem chess, int tournament_id, int first_player,
                         int second_player, Winner winner, int play_time)
{
    if(chess == NULL){
        return CHESS_NULL_ARGUMENT;
    }
    if(first_player < MIN || second_player < MIN || first_player == second_player || tournament_id <MIN){
        return CHESS_INVALID_ID;
    }
    bool first_player_was_allocated = false , second_player_was_allocated =false ;
    PlayerNodeResult result_players = playersAdd(chess->external_players , first_player , second_player ,
                                                 &first_player_was_allocated, &second_player_was_allocated);
    if(result_players == PLAYER_MEMORY_FAILED){
        return CHESS_OUT_OF_MEMORY;
    }
    TournamentResult result = tournamentAddGame(chess->tournaments , tournament_id , first_player,
                                                second_player , winner , play_time);
    if(result != TOUR_SUCCESS)
    {
        if(first_player_was_allocated){
            mapRemove(chess->external_players, &first_player);
        }
        if(second_player_was_allocated){
            mapRemove(chess->external_players, &second_player);
        }
        return convertTourToChess(result);
    }
    updateExternalPlayer(chess->external_players , winner , first_player , second_player);
    return CHESS_SUCCESS;
}


ChessResult chessRemoveTournament (ChessSystem chess, int tournament_id){
    if(chess == NULL)
    {
        return CHESS_NULL_ARGUMENT;
    }
    if(tournament_id < MIN)
    {
        return CHESS_INVALID_ID;
    }
    if(mapContains(chess->tournaments , &tournament_id) == false)
    {
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
    tournamentDestroy(chess->tournaments , chess->external_players , tournament_id);
    return CHESS_SUCCESS;
}

ChessResult chessEndTournament(ChessSystem chess, int tournament_id){
    if(chess == NULL)
    {
        return CHESS_NULL_ARGUMENT;
    }
    if(tournament_id < MIN)
    {
        return CHESS_INVALID_ID;
    }
    if(mapContains(chess->tournaments , &tournament_id) == false)
    {
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
    TournamentResult result = tournamentEnd (chess->tournaments , tournament_id);
    if(result == TOUR_ENDED)
    {
        return CHESS_TOURNAMENT_ENDED;
    }
    if(result == TOUR_N0_GAMES)
    {
        return CHESS_NO_GAMES;
    }
    return CHESS_SUCCESS;
}

double chessCalculateAveragePlayTime (ChessSystem chess, int player_id, ChessResult *chess_result){
    if(chess_result == NULL){
        return NOT_VALID;
    }
    if(chess == NULL){
        *chess_result = CHESS_NULL_ARGUMENT;
        return NOT_VALID;
    }
    if(player_id < MIN){
        *chess_result = CHESS_INVALID_ID;
        return NOT_VALID;
    }
    if(mapContains(chess->external_players , &player_id) == false){
        *chess_result = CHESS_PLAYER_NOT_EXIST;
        return NOT_VALID;
    }
    int num_of_games = playerGetNumOfGames(chess->external_players, &player_id);
    if(num_of_games == NO_GAMES)
        return NO_GAMES;
    double average_time = tournamentCalculateTotalTime (chess->tournaments , player_id);
    *chess_result = CHESS_SUCCESS;
    return average_time / (num_of_games);
}

ChessResult chessSavePlayersLevels (ChessSystem chess, FILE* file){
    if(chess == NULL || file == NULL)
    {
        return CHESS_NULL_ARGUMENT;
    }
    double level;
    PlayerNodeResult result;
    playerInitiateStatus(chess->external_players);
    int player_to_print = playerGetPlayerByLevels(chess->external_players , &level , &result);
    if(player_to_print == DUMMY_LIMB_ID){
        return PlayerLevelsResult(result);
    }
    while (player_to_print != DUMMY_LIMB_ID){
        if(fprintf(file,"%d %.2lf\n", player_to_print , level) < MIN_PRINT_SUCCESS)
        {
            return CHESS_SAVE_FAILURE;
        }

        player_to_print = playerGetPlayerByLevels(chess->external_players , &level , &result);
    }
    if(result == PLAYER_MEMORY_FAILED){
        return CHESS_OUT_OF_MEMORY;
    }
    playerResetStatus(chess->external_players);
    return CHESS_SUCCESS;
}

ChessResult chessSaveTournamentStatistics(ChessSystem chess, char* path_file){
    if(chess == NULL || path_file == NULL)
    {
        return CHESS_NULL_ARGUMENT;
    }
    if(tournamentExistEnded(chess->tournaments) == false){
        return CHESS_NO_TOURNAMENTS_ENDED;
    }
    TournamentResult result = tournamentStatistic(path_file , chess->tournaments);
    if(result == TOUR_MEMORY_PROBLEM)
    {
        return CHESS_SAVE_FAILURE;
    }
    return CHESS_SUCCESS;
}

ChessResult chessRemovePlayer(ChessSystem chess, int player_id){
    if(chess == NULL)
    {
        return CHESS_NULL_ARGUMENT;
    }
    if(player_id < MIN)
    {
        return CHESS_INVALID_ID;
    }
    assert(chess->external_players != NULL);
    if(mapRemove(chess->external_players , &player_id) == MAP_ITEM_DOES_NOT_EXIST)
    {
        return CHESS_PLAYER_NOT_EXIST;
    }
    assert(chess->tournaments != NULL);
    tournamentRemovePlayer(chess->tournaments , chess->external_players , player_id);
    return CHESS_SUCCESS;
}
//static functions

static bool isTourLocationValid(const char* tournament_location) {
    assert(tournament_location != NULL);
    if (*tournament_location < 'A' || *tournament_location > 'Z')
    {
        return false;
    }
    for (int i = 1; i < *(tournament_location+i); ++i) {
        if((*(tournament_location+i) >= 'a' && *(tournament_location+i) <= 'z') ||
           *(tournament_location+i) == ' ')
        {
            continue;
        }
        return false;
    }
    return true;
}

static ChessResult convertTourToChess(TournamentResult tour_result)
{
    if(tour_result == TOUR_NOT_EXIST)
    {
        return CHESS_TOURNAMENT_NOT_EXIST;
    }
    if(tour_result == TOUR_ENDED)
    {
        return CHESS_TOURNAMENT_ENDED;
    }
    if(tour_result == TOUR_EXIST_GAME)
    {
        return CHESS_GAME_ALREADY_EXISTS;
    }
    if(tour_result == TOUR_NEGATIVE_TIME)
    {
        return CHESS_INVALID_PLAY_TIME;
    }
    if(tour_result == TOUR_EXCEEDED_GAMES)
    {
        return CHESS_EXCEEDED_GAMES;
    }
    return CHESS_OUT_OF_MEMORY;
}

static ChessResult PlayerLevelsResult(PlayerNodeResult result){
    if(result == PLAYER_NULL_ARGUMENT){
        return CHESS_NULL_ARGUMENT;
    }
    if(result == PLAYER_MEMORY_FAILED){
        return CHESS_OUT_OF_MEMORY;
    }
    if(result == PLAYER_NO_PLAYERS_TO_PRINT){
        return CHESS_SUCCESS;
    }
    return CHESS_SUCCESS;
}

static ChessResult chessAddGamesResults(ChessSystem chess, int tournament_id ,
                                        int max_games_per_player,
                                        const char* tournament_location){
    assert(chess != NULL || tournament_location != NULL);
    if(tournament_id < MIN)
    {
        return CHESS_INVALID_ID;
    }
    if(mapContains(chess->tournaments , &tournament_id) == true)
    {
        return CHESS_TOURNAMENT_ALREADY_EXISTS;
    }
    if(isTourLocationValid(tournament_location) == false)
    {
        return CHESS_INVALID_LOCATION;
    }
    if(max_games_per_player < MIN)
    {
        return CHESS_INVALID_MAX_GAMES;
    }
    return CHESS_SUCCESS;
}
