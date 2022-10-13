//
// Created by aribr on 07/05/2021.
//

#ifndef MTM_CHESS_PLAYERS_H
#define MTM_CHESS_PLAYERS_H

#include "map.h"
#include <stdbool.h>
#include "chessSystem.h"

typedef struct player_t *PlayerData;

/** Type used for returning error codes from playerData functions */

typedef enum PlayerNodeResult_t{
    PLAYER_SUCCESS,
    PLAYER_MEMORY_FAILED,
    PLAYER_NULL_ARGUMENT,
    PLAYER_NO_PLAYERS_TO_PRINT
}PlayerNodeResult;

/** Type used for sending different flags to playerData functions */

typedef enum PlayerFlag_t{
    PLAYER_WIN,
    PLAYER_LOSS,
    PLAYER_DRAWS,
    PLAYER_LEVEL_COMPARE,
    PLAYER_POINTS_COMPARE
}PlayerFlag;

/** function for creating a playerNode
 * Initializes all int fields to 0
 * @return
 * NULL - if allocation error
 * playerNode - otherwise
 */

PlayerData playerCreatNode();

/**
 * function for creating a player map
 * @return
 * NULL - if memory failed
 * players map - otherwise
 */
Map playersCreateMap();

/**compares between 2 players who has a greater level or points:
 * @param player_list - the list in which the players are
 * @param player_id1
 * @param player_id2
 * @param compare_type - the flag of Comparison (PLAYER_LEVEL_COMPARE or PLAYER_POINTS_COMPARE)
 * @return
*  A positive integer if the first element is greater;
*  0 if PLAYER_NULL_ARGUMENT or equal;
*  A negative integer if the second element is greater.
 */

int playerCompare(Map players, MapKeyElement player_id1 , MapKeyElement player_id2 ,
                  PlayerFlag const *compare_type);
/**
 * compares between 2 int keys:
 * @param key1
 * @param key2
 * @return
*  A positive integer if the first element is greater;
*  0  - if PLAYER_NULL_ARGUMENT ;
*  A negative integer if the second element is greater.
 */

int intKeyCompare(MapKeyElement key1 , MapKeyElement key2 );

/**
 * function for getting the winner of the list
 * @param playerList;
 * @return
 * -1 - if NULL map was sent
 * 0 - no players at the tournament
 * the key of the winner - otherwise
 */
int playerGetTournamentWinnerID(Map player);
/**
 * function for getting the number of games of a player
 * @param playerList;
 * @param id - a pointer to the id Of the player for whom the information is requested
 * @return num of games , -1 for NULL, 0 if player not exist
 */

int playerGetNumOfGames(Map players, MapKeyElement player_id);

/** function for adding new data for a player
 * @param id - the id Of the player for whom the update is requested result
 * @param player_list
 * @param result  - type of update ( WIN , LOSS , DRAW)
 * @param factor - factor of adding or removing data
 * @return
 * PLAYER_NULL_ARGUMENT - if a NULL player list was sent
 * PLAYER_SUCCESS - otherwise
 */

PlayerNodeResult playerUpdateData(Map players, MapKeyElement player_id,
                                  PlayerFlag result, int factor);
/**
 * playerGetPlayerByLevels: getting the id of the player with the highest level and non printed status
 * @param players - list that contains the players
 * @param level - a pointer where to put the level of the above player
 * @param result - a pointer where to put the result - PLAYER_NULL_ARGUMENT if a null was sent,
 * PLAYER_SUCCESS otherwise
 * @return
 * -1 - if player list is NULL or no more player level to print
*  player id - the id of the player with the highest level and non printed status
 */

int playerGetPlayerByLevels(Map players, double *level , PlayerNodeResult *result);

/**
 * playerResetStatus - resets all player's status to UNPRINTED
 * @param player_list - the list of players to reset
 */

void playerResetStatus(Map players);

/**
 * playerRemoveData: removes all the data of the external list according to the internal list
 * @param external_list - the system's player list
 * @param internal_list - the tournament's player list
 * @return
 * PLAYER_NULL_ARGUMENT - if external_list or internal_list are NULL
 * CHESS_SUCCESS - otherwise
 */

PlayerNodeResult playerRemoveData(Map external_list, Map internal_list);

/**
 *
 * @param players - map of players
 * @param first_player - first player to add
 * @param second_player - second player to add
 * @param first_player_was_allocated - a pointer to bool that checks if fist player was allocated
 * @param second_player_was_allocated - a pointer to bool that checks if fist player was allocated
 * @return
 * PLAYER_MEMORY_FAILED - if allocation error
 * PLAYER_SUCCESS - otherwise
 */
PlayerNodeResult playersAdd (Map players , int first_player , int second_player ,
                             bool *first_player_was_allocated , bool *second_player_was_allocated);

/**
 *
 * @param players - map of players
 * @param winner - indicates the winner in the match. if it is FIRST_PLAYER, then the first player won.
 *                 if it is SECOND_PLAYER, then the second player won, otherwise the match has ended with
 *                 a draw.
 * @param first_player - first player to update
 * @param second_player - second player to update
 */

void updateExternalPlayer(Map players , Winner winner ,
                                      int first_player,int second_player);
/**
 * function for coping a key element
 * @param player_id - the key which to copy
 * @return
 * NULL - if allocation error or null argument sent
 * copy of the key - otherwise
 */

MapKeyElement copyIntKey(MapKeyElement key);

/**
 * function for deallocating a int key
 * @param key - key to free
 */

void destroyIntKey(MapKeyElement key);
/**
 * function for deallocating a Node
 * @param Node - Node to free
 */
void destroyNode(MapDataElement Node);

/**
 * function for initiating all players' status with 0 number of games to PRINTED
 * @param players - players to status initiate
 */

void playerInitiateStatus(Map players);
#endif //MTM_CHESS_PLAYERS_H
