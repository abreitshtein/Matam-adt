#include "games.h"
#include "chessSystem.h"
#ifndef MTM_3_2_TOURNAMENT_H
#define MTM_3_2_TOURNAMENT_H

typedef struct tour_data *TourData;

typedef enum ChessTournament_t {
    TOUR_MEMORY_PROBLEM,
    TOUR_SUCCESS,
    TOUR_EXCEEDED_GAMES,
    TOUR_EXIST_GAME,
    TOUR_NEGATIVE_TIME,
    TOUR_ENDED,
    TOUR_NOT_EXIST,
    TOUR_N0_GAMES

}TournamentResult;

/**
* tournamentCreateNew: create a new tournament map with his own function.
*
* @return
 * 	Null - allocation failed.
 * 	pointer to the map - allocation success.
*/
Map tournamentCreateNew();
/**
* tournamentAddGame: Added a new tournament to the system.
*
* @param tournament - pointer to the tournament map.
* @param tournament_id - the id of the tournament.
* @param location - pointer to const char that has the location of the tournament.
* @param max_games - the max game that allowed in this tournament per player.
* @return
*  TOUR_MEMORY_PROBLEM - allocation failed.
 * 	TOUR_SUCCESS - the tour added successfully.
*/
TournamentResult tournamentAdd(Map tournament, int tournament_id,
                               const char* location, int max_games);

/**
* tournamentAddGame: Added a new game to the tournament.
*
* @param tournament - pointer to the tournament map.
* @param tournament_id - the id of the tournament.
* @param id1 - the id of the first player.
* @param id2 - the id of the second player.
* @param winner - the id of the winner, if its a tide will be -1.
* @param game_time - the time of the game.
* @return
 *  TOUR_NOT_EXIST - there is no tournament with this id
*   TOUR_ENDED - if the tournament end.
*   TOUR_EXIST_GAME - already exist a game in this tournament with this 2 id.
*   TOUR_NEGATIVE_TIME - the time is negative.
* 	TOUR_EXCEEDED_GAMES - one of the players has exceeded the max number of game.
* 	TOUR_MEMORY_PROBLEM - allocation failed.
* 	TOUR_SUCCESS - the game added successfully.
*/
TournamentResult tournamentAddGame(Map tournament, int tournament_id, int id1,
                                   int id2, Winner winner, int game_time);
/**
* tournamentStatistic: Added a statistics about ended tournament to a file.
*
* @param path_file - the location of the file which the data will be add.
* @param tournament - pointer to the tournament map.
* @return
* 	TOUR_MEMORY_PROBLEM - file opening failed or failing during save.
* 	TOUR_SUCCESS - the data added successfully.
*/
TournamentResult tournamentStatistic(char* path_file, Map tournament);
/**
* tournamentDestroy: destroy a tournament and deallocate its all memory.
*
* @param tournament - pointer to the tournament map.
* @param external_player - pointer to the player external map.
* @param tour_id - this id of the tournament.
* @return
* 	TOUR_MEMORY_PROBLEM - file opening failed or failing during save.
* 	TOUR_SUCCESS - the data added successfully.
*/
void tournamentDestroy(Map tournament, Map external_player, int id);
/**
* tournamentRemovePlayer: remove a player and its data from all the tournaments.
*
* @param tournament - pointer to the tournament map.
* @param external_player - pointer to the external map of the players.
* @param player_id - the id of the player which we want to remove.
* @return
* 	TOUR_NULL_ARGUMENT - NULL pointers was sent.
 * 	TOUR_SUCCESS - deleted successfully.
*/
void tournamentRemovePlayer(Map tournament, Map external_player, int player_id);
/**
* tournamentEnd: close the tournament and find the winner.
*
* @param tournament - pointer to the tournament map.
* @param player_id - the id of the player which we want to remove.
* @return
 * 	TOUR_ENDED - if the tournament ended.
 * 	TOUR_N0_GAMES - there is no games in the tournament.
 * 	TOUR_SUCCESS - closed successfully.
*/
TournamentResult tournamentEnd(Map tournament, int tour_id);
/**
* tournamentCalculateTotalTime: calculate the total time that a player has been played.
* @return
* 	the time.
*/
double tournamentCalculateTotalTime(Map tournament, int player_id);
/**
* touramentExistEnded: checks if there is closed tournament.
*
* @param tournament - pointer to the tournaments map
* @return
* 	true - there is.
 * 	false - there is not.
*/
bool tournamentExistEnded(Map tournament);



#endif //MTM_3_2_TOURNAMENT_H
