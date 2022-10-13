//
// Created by aribr on 07/05/2021.
//
#include "players.h"
#include <stdlib.h>
#include "assert.h"
#include "stdbool.h"

#define WIN_COEFFICIENT 6
#define LOSS_COEFFICIENT 10
#define DRAW_COEFFICIENT 2
#define WIN_WEIGHT 2
#define NULL_ARGUMENT 0
#define EMPTY (-1)
#define FIRST_IS_GREATER 1
#define SECOND_IS_GREATER (-1)
#define NO_GAMES 0
#define EQUAL 0
#define PRINTED 1
#define UNPRINTED 0
#define ADD 1

struct player_t{
    int winning;
    int loss;
    int draw;
    int num_of_games;
    bool status;
};



// Updates multiple wins or losses or  draws at once
static void updatePlayerByFactor(PlayerFlag result , PlayerData player, int factor);

//Calculates player's level according to the given formula and returns the level
static double calculateLevel(PlayerData player);

//compares between 2 players by level
//returns:
//  A positive integer if the first element is greater;
//  0 if PLAYER_NULL_ARGUMENT or equal;
// A negative integer if the second element is greater.

static int playerCompareLevel(PlayerData player1 , PlayerData player2);

//compares between 2 players by points
//returns:
//  A positive integer if the first element is greater;
//  0 if PLAYER_NULL_ARGUMENT or equal;
// A negative integer if the second element is greater.

static int playerComparePoints(PlayerData player1 , PlayerData player2);

//function for coping a player data
// returns NULL if allocation error , the playerData copy otherwise
static MapDataElement copyPlayerData(MapDataElement player_data);


//function for getting the first UNPRINTED player
//returns PLAYER_NO_PLAYERS_TO_PRINT if no more players
// to print or the first player to compare otherwise

static int playerGetFirstToCompare(Map players , PlayerNodeResult *result);

// function for deallocating data while adding new players

static void deallocateData(Map players , PlayerData first_player_data ,
                           PlayerData second_player_data , int first_player ,
                           int second_player , bool const *first_player_was_allocated ,
                           bool const *second_player_was_allocated);


Map playersCreateMap(){
    Map players = mapCreate(copyPlayerData , copyIntKey , destroyNode ,
                            destroyIntKey , intKeyCompare);
    if(players == NULL){
        return NULL;
    }
    return players;
}

MapKeyElement copyIntKey(MapKeyElement key){
    if(key == NULL){
        return NULL;
    }
    int *new_key = (int*)malloc(sizeof (int));
    if(new_key == NULL){
        return NULL;
    }
    *new_key = *(int*)key;
    return new_key;
}

PlayerData playerCreatNode(){
    PlayerData new_player_data = malloc(sizeof(*new_player_data));
    if(new_player_data == NULL)
    {
        return NULL;
    }
    new_player_data->winning = NULL_ARGUMENT;
    new_player_data->loss = NULL_ARGUMENT;
    new_player_data->draw = NULL_ARGUMENT;
    new_player_data->num_of_games = NULL_ARGUMENT;
    new_player_data->status = NULL_ARGUMENT;
    return new_player_data;
}

void destroyNode(MapDataElement player_data){
    if(player_data != NULL){
        free(player_data);
    }
}

void destroyIntKey(MapKeyElement key){
    if(key != NULL){
        free(key);
    }
}


int playerCompare(Map players, MapKeyElement player_id1 , MapKeyElement player_id2 ,
                  PlayerFlag const *compare_type){
    assert(players != NULL && player_id1 != NULL && player_id2 != NULL && compare_type != NULL);
    PlayerData player1_data = (PlayerData) mapGet(players , player_id1);
    PlayerData player2_data = (PlayerData) mapGet(players , player_id2);
    if(player1_data == NULL || player2_data == NULL){
        return NULL_ARGUMENT;
    }
    assert(*compare_type == PLAYER_LEVEL_COMPARE || *compare_type == PLAYER_POINTS_COMPARE);
    int result;
    if(*compare_type == PLAYER_POINTS_COMPARE){
       result = playerComparePoints(player1_data , player2_data);
    } else{
        result = playerCompareLevel(player1_data , player2_data);
    }
    assert(result != EQUAL);
    if(result > EQUAL){
        return FIRST_IS_GREATER;
    } else{
        return SECOND_IS_GREATER;
    }
}

int intKeyCompare(MapKeyElement player_id1 , MapKeyElement player_id2 ){
    assert(player_id1 != NULL && player_id2 != NULL);
    if(*(int*)player_id1 > *(int*)player_id2){
        return FIRST_IS_GREATER;
    }
    if(*(int*)player_id1 < *(int*)player_id2){
        return SECOND_IS_GREATER;
    }
    return EQUAL;
}


int playerGetTournamentWinnerID(Map players){
    if(players == NULL){
        return EMPTY;
    }
    MapKeyElement tournament_winner = mapGetFirst(players);
    if(tournament_winner == NULL){
        return NULL_ARGUMENT;
    }
    MAP_FOREACH(int*, iterator,players){
        PlayerFlag compare_type = PLAYER_POINTS_COMPARE;
        if(playerCompare(players , tournament_winner , iterator , &compare_type) < EQUAL){
            *(int*) tournament_winner = *(int*) iterator;
        }
        destroyIntKey(iterator);
    }
    int temp = *(int*)tournament_winner;
    destroyIntKey(tournament_winner);
    return temp;
}

int playerGetNumOfGames(Map players, MapKeyElement player_id){
    if(players == NULL){
        return EMPTY;
    }
    PlayerData player_data = (PlayerData) mapGet(players , player_id);
    if(player_data == NULL){
        return NULL_ARGUMENT;
    }
    return player_data->num_of_games;
}

PlayerNodeResult playerUpdateData(Map players, MapKeyElement player_id,
                                          PlayerFlag result, int factor){
    if(players == NULL || player_id == NULL){
        return PLAYER_NULL_ARGUMENT;
    }
    assert(mapContains(players ,player_id) == true);
    PlayerData player_data = mapGet(players , player_id);
    assert(player_data != NULL);
    updatePlayerByFactor(result, player_data , factor);
    return PLAYER_SUCCESS;
}

int playerGetPlayerByLevels(Map players, double *level , PlayerNodeResult *result){
    assert(level != NULL && result != NULL);
    if(players == NULL){
        *result = PLAYER_NULL_ARGUMENT;
        return EMPTY;
    }
    PlayerNodeResult result1 = PLAYER_SUCCESS;
    int highest_level_player = playerGetFirstToCompare(players , &result1);
    if(result1 != PLAYER_SUCCESS){
        assert(highest_level_player == EMPTY);
        *result = result1;
        return EMPTY;
    }
    MAP_FOREACH(int*, iterator,players){
        PlayerFlag compare_type = PLAYER_LEVEL_COMPARE;
        PlayerData temp = (PlayerData) mapGet(players , iterator);
        assert(temp != NULL);
        if(playerCompare(players , &highest_level_player , iterator , &compare_type) < EQUAL){
            if(temp->status == UNPRINTED){
                highest_level_player = *(int*) iterator;
            }
        }
        destroyIntKey(iterator);
    }
    PlayerData player_data = mapGet(players , &highest_level_player);
    assert(player_data != NULL);
    player_data->status = PRINTED;
    *level = calculateLevel(player_data);
    *result = PLAYER_SUCCESS;
    return highest_level_player;
}

void playerInitiateStatus(Map players){
    assert(players != NULL);
    MAP_FOREACH(int*, iterator,players){
        PlayerData player_data = (PlayerData) mapGet(players , iterator);
        assert(player_data != NULL);
        if(player_data->num_of_games == NO_GAMES){
            player_data->status = PRINTED;
        }
        destroyIntKey(iterator);
    }
}

void playerResetStatus(Map players){
    assert(players != NULL);
    MAP_FOREACH(int*, iterator,players){
        PlayerData player_data = (PlayerData) mapGet(players , iterator);
        player_data->status = UNPRINTED;
        destroyIntKey(iterator);
    }
}

PlayerNodeResult playerRemoveData(Map external_list, Map internal_list){
    if(internal_list == NULL || external_list == NULL){
        return PLAYER_NULL_ARGUMENT;
    }
    MAP_FOREACH(int*, iterator,internal_list){
        PlayerData internal_player_data = (PlayerData) mapGet(internal_list , iterator);
        if(internal_player_data == NULL){
            return PLAYER_NULL_ARGUMENT;
        }
        playerUpdateData(external_list , iterator ,PLAYER_WIN,
                                  -internal_player_data->winning);
        playerUpdateData(external_list , iterator ,PLAYER_LOSS,
                                  -internal_player_data->loss);
        playerUpdateData(external_list , iterator ,PLAYER_DRAWS,
                                  -internal_player_data->draw);
        destroyIntKey(iterator);
    }
    return PLAYER_SUCCESS;
}


void updateExternalPlayer(Map players , Winner winner ,
                                             int first_player,int second_player){
    assert(players != NULL);
    assert(mapContains(players , &first_player) == true);
    assert(mapContains(players , &second_player) == true);
    if(winner == FIRST_PLAYER){
        playerUpdateData(players , &first_player , PLAYER_WIN , ADD);
        playerUpdateData(players , &second_player , PLAYER_LOSS , ADD);
    }
    if(winner == SECOND_PLAYER){
        playerUpdateData(players , &first_player , PLAYER_LOSS , ADD);
        playerUpdateData(players , &second_player , PLAYER_WIN , ADD);
    }
    if(winner == DRAW){
        playerUpdateData(players , &first_player , PLAYER_DRAWS , ADD);
        playerUpdateData(players , &second_player , PLAYER_DRAWS , ADD);
    }
}


PlayerNodeResult playersAdd (Map players , int first_player , int second_player ,
                                 bool *first_player_was_allocated , bool *second_player_was_allocated)
{
    assert(first_player_was_allocated != NULL && second_player_was_allocated != NULL && players != NULL);
    PlayerData first_player_data = NULL , second_player_data = NULL;
    if(mapContains(players , &first_player ) == false){
        first_player_data = playerCreatNode();
        if(first_player_data == NULL){
            return PLAYER_MEMORY_FAILED;
        }
        if(mapPut(players , &first_player , first_player_data) == MAP_OUT_OF_MEMORY){
            destroyNode(first_player_data);
            return PLAYER_MEMORY_FAILED;
        }
        *first_player_was_allocated = true;
    }
    if(mapContains(players , &second_player) == false){
        second_player_data = playerCreatNode();
        if(second_player_data == NULL){
            deallocateData(players , first_player_data , second_player_data ,
                           first_player , second_player , first_player_was_allocated ,
                           second_player_was_allocated);
            return PLAYER_MEMORY_FAILED;
        }
        if(mapPut (players , &second_player , second_player_data) == MAP_OUT_OF_MEMORY){
            deallocateData(players , first_player_data , second_player_data ,
                           first_player , second_player , first_player_was_allocated,
                           second_player_was_allocated);
            return PLAYER_MEMORY_FAILED;
        }
        *second_player_was_allocated = true;
    }
    destroyNode(first_player_data);
    destroyNode(second_player_data);
    return PLAYER_SUCCESS;
}



//static functions:

static MapDataElement copyPlayerData(MapDataElement player_data){
    PlayerData new_player_data = malloc(sizeof(*new_player_data));
    if(new_player_data == NULL)
    {
        return NULL;
    }
    PlayerData temp_player_data = (PlayerData)player_data;
    new_player_data->winning = temp_player_data->winning;
    new_player_data->loss = temp_player_data->loss;
    new_player_data->draw = temp_player_data->draw;
    new_player_data->num_of_games = temp_player_data->num_of_games;
    new_player_data->status = temp_player_data->status;
    return new_player_data;
}

static int playerComparePoints(PlayerData player1 , PlayerData player2){
    assert(player1 != NULL && player2 != NULL);
    int points_of_player1 = WIN_WEIGHT * player1->winning + player1->draw;
    int points_of_player2 = WIN_WEIGHT * player2->winning + player2->draw;
    if(points_of_player1 > points_of_player2)
    {
        return FIRST_IS_GREATER;
    }
    if(points_of_player1 < points_of_player2)
    {
        return SECOND_IS_GREATER;
    }
    //else, pointsOfPlayer1 == pointsOfPlayer2
    if(player1->loss < player2->loss)
    {
        return FIRST_IS_GREATER;
    }
    if(player1->loss > player2->loss)
    {
        return SECOND_IS_GREATER;
    }
    //else, player1->losses == player2->losses
    if(player1->winning > player2->winning)
    {
        return FIRST_IS_GREATER;
    }
    if(player1->winning < player2->winning)
    {
        return SECOND_IS_GREATER;
    }
    //else, player1->Winnings == player2->Winnings
    return FIRST_IS_GREATER;
}

static int playerCompareLevel(PlayerData player1 , PlayerData player2){
    assert(player1 != NULL || player2 != NULL);
    double player1_level , player2_level;
    if(player1->num_of_games == NO_GAMES){
        player1_level = NO_GAMES;
    } else{
        player1_level = calculateLevel(player1);
    }
    if(player2->num_of_games == NO_GAMES){
        player2_level = NO_GAMES;
    } else{
        player2_level = calculateLevel(player2);
    }
    if(player1_level > player2_level){
        return FIRST_IS_GREATER;
    }
    if(player1_level < player2_level)
    {
        return SECOND_IS_GREATER;
    }
    return FIRST_IS_GREATER;
}

static int playerGetFirstToCompare(Map players , PlayerNodeResult *result){
    int first_to_compare = EMPTY;
    MAP_FOREACH(int*, iterator,players){
        PlayerData player_data = (PlayerData) mapGet(players , iterator);
        assert(player_data != NULL);
        if(player_data->status == UNPRINTED){
            first_to_compare = *(int*) iterator;
            destroyIntKey(iterator);
            break;
        }
        destroyIntKey(iterator);
    }
    if(first_to_compare == EMPTY){
        *result = PLAYER_NO_PLAYERS_TO_PRINT;
        return EMPTY;
    }
    return first_to_compare;
}

static void deallocateData(Map players , PlayerData first_player_data ,
                           PlayerData second_player_data , int first_player ,
                           int second_player , bool const *first_player_was_allocated ,
                           bool const *second_player_was_allocated){
    destroyNode(first_player_data);
    destroyNode(second_player_data);
    if(*first_player_was_allocated == true){
        mapRemove(players , &first_player);
    }
    if(*second_player_was_allocated == true){
        mapRemove(players , &second_player);
    }
}

static double calculateLevel(PlayerData player_data){
    assert(player_data != NULL);
    assert(player_data->num_of_games != EQUAL);
    return (double)((WIN_COEFFICIENT * player_data->winning) -
                    (LOSS_COEFFICIENT * player_data->loss) +
                    (DRAW_COEFFICIENT * player_data->draw)) / (player_data->num_of_games);
}

static void updatePlayerByFactor(PlayerFlag result,
                                 PlayerData player, int factor){
    assert(player != NULL);
    if(result == PLAYER_WIN){
        player->winning += factor;
    }
    if(result == PLAYER_LOSS){
        player->loss += factor;
    }
    if(result == PLAYER_DRAWS){
        player->draw += factor;
    }
    player->num_of_games += factor;
    assert(player->num_of_games>=0);
}