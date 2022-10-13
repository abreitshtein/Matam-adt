#include "stdbool.h"
#include "players.h"
#ifndef GAMES_H
#define GAMES_H

typedef struct GameData_t *GameData;
typedef struct Map_Head *MapHead;
typedef enum MapGameResult_t {
    GAME_OUT_OF_MEMORY,
    GAME_SUCCESS,
    GAME_NULL_ARGUMENT
} GameResult;
typedef enum MapGameInformation_t {
    GAME_NUM_PLAYERS,
    GAME_NUM_GAMES,
    GAME_LONGEST,
} GameInformation;


/**
* gameCreate: Allocates a new game Map.
*
* @return
*   pointer to the map - the allocation succeeded.
* 	NULL - allocations failed.
*/
MapHead gameCreate();
/**
* gameCopy: copy a game map
*
* @param games - pointer to the game map.
* @return
*   pointer to the new map - success.
* 	NULL - allocations failed.
*/
MapHead gameCopy(MapHead game);
/**
* gameDestroy: destroy a game map.
*
* @param games - pointer to the game map.
* @return
 * nothing.
*/
void gameDestroy(MapHead game);
/**
* gameAdd: add a new game to the map.
*
* @param games - pointer to the game map.
* @param players - pointer to the players map.
* @param first_id - the id of the first player.
* @param second_id - the id of the second player.
* @param winner - enum of the player that won.
* @param play_time - the time that the game took.
* @return
 * GAME_OUT_OF_MEMORY - allocation failed.
 * GAME_SUCCESS - added successfully.
*/
GameResult gameAdd(MapHead game, Map players, int first_id, int second_id, Winner winner, int play_time);
/**
* gameUpdateQuitPlayer: Update the games that the quit player was played,
 * updated the scores in the player struct, change the winner game.
 *
* @param games - pointer to the game map.
 * @param internal_player - pointer to the players map.
  * @param external_player - pointer to the  external players map.
 * @param id_quit_player - the id of the quit player.
 * assert that all the data are correct.
 * @return
 * nothing.
*/
void gameUpdateQuitPlayer(MapHead game, Map internal_player,
                          Map external_player, int id_quit_player);
/**
* gameExist: chekc if there is game with this 2 id.
* @param game - pointer to the game map.
* @param first_id - the id of the first player.
* @param second_id - the id of the second player.
 * @return
 * true if there is. false if not.
*/
bool gameExist(MapHead game, int first_id, int second_id);
/**
* gamePlayedTime: return the total time that this id played
* @param game - pointer to the map games.
* @param id - the id of the player
* @return
* 	the total time
*/
double gamePlayedTime(MapHead game, int id);
/**
* gameAvgTime: return the average play time.
* @param game - pointer to the map games.
* @return
* 	the average time.
*/
double gameAvgTime(MapHead game);
/**
* gameGetInfo: return a info about the games.
* @param game - pointer to the map games.
* @return
* 	GAME_NUM_PLAYERS - the number of the players.
 * 	GAME_NUM_GAMES - the number of the games.
 * 	GAME_LONGEST - the longest game.
*/
int gameGetInfo(MapHead game, GameInformation info);
#endif //MTM_3_2_GAMES_H
