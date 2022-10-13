#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "map.h"
#define EQUAL 0

typedef struct node_t{
    MapKeyElement keyElement;
    MapDataElement DataElement;
    struct node_t *next;
}*MapNode;
typedef struct MapFunctions{
    copyMapDataElements CopyDataElement;
    copyMapKeyElements CopyKeyElement;
    freeMapDataElements FreeMapDataElement;
    freeMapKeyElements FreeMapKeyElement;
    compareMapKeyElements CompareKeyElement;
}Functions;

struct Map_t{
    Functions MapFunc;
    MapNode head;
    MapNode iterator;
};
/* add a new key to the map.
 * return MAP_SUCCESS if added, or MAP_OUT_OF_MEMORY if there was allocation error */
static MapResult addNewKey(Map map, MapKeyElement keyElement, MapDataElement dataElement);
/* update an exist key with a new data.
 * return MAP_SUCCESS if updated successfully, or MAP_OUT_OF_MEMORY if there was allocation error */
static MapResult updateMapKey(Map map, MapKeyElement keyElement, MapDataElement dataElement);
/* update a map node with a data using map copy function */
static void updateMapNode(Map map, MapNode new, MapNode original);
/* deallocate the key, data and the node of the map. */
static void freeMapNode(Map map, MapNode to_delete);
/* check if the allocation of a new node failed during a copy map.
 * if yes, remove the map and return true. if no return false */
static bool checkAllocationFailed(Map copied_map, MapNode new);

Map mapCreate(copyMapDataElements copyDataElement,
              copyMapKeyElements copyKeyElement,
              freeMapDataElements freeDataElement,
              freeMapKeyElements freeKeyElement,
              compareMapKeyElements compareKeyElements)
{
    if(copyDataElement == NULL || copyKeyElement == NULL || freeDataElement == NULL||
       freeKeyElement == NULL || compareKeyElements == NULL) {
        return NULL;
    }
    Map new_map = (Map)malloc(sizeof(*new_map));
    if(new_map == NULL) {
        return NULL;
    }
    new_map->head = NULL;
    new_map->iterator = NULL;
    new_map->MapFunc.CopyDataElement = copyDataElement;
    new_map->MapFunc.CopyKeyElement = copyKeyElement;
    new_map->MapFunc.FreeMapDataElement = freeDataElement;
    new_map->MapFunc.FreeMapKeyElement = freeKeyElement;
    new_map->MapFunc.CompareKeyElement = compareKeyElements;
    return new_map;
}
void mapDestroy(Map map)
{
    if(mapClear(map) == MAP_NULL_ARGUMENT) {
        return;
    }
    free(map);
}
Map mapCopy(Map map)
{
    if(map == NULL) {
        return NULL;
    }
    Map copied_map = mapCreate(map->MapFunc.CopyDataElement,map->MapFunc.CopyKeyElement,
                               map->MapFunc.FreeMapDataElement,map->MapFunc.FreeMapKeyElement,
                               map->MapFunc.CompareKeyElement);
    if(copied_map == NULL) {
        return NULL;
    }
    if(map->head == NULL) {
        return copied_map;
    }
    MapNode copied_head = malloc(sizeof(*copied_head));
    if(checkAllocationFailed(copied_map, copied_head)){
        return NULL;
    }
    copied_map->head = copied_head;
    MapNode original_head = map->head;
    updateMapNode(map, copied_head, original_head);
    original_head = original_head->next;
    MapNode tail = copied_map->head;
    while (original_head != NULL)
    {
        MapNode copied_new = malloc(sizeof(*copied_new));
        if(checkAllocationFailed(copied_map, copied_new)){
            return NULL;
        }
        updateMapNode(map, copied_new, original_head);
        tail->next = copied_new;
        tail = copied_new;
        original_head = original_head->next;
    }
    return copied_map;
}
int mapGetSize(Map map)
{
    if(map == NULL) {
        return -1;
    }
    int count = 0;
    MapNode helper = map->head;
    while (helper != NULL)
    {
        count++;
        helper = helper->next;
    }
    return count;
}
bool mapContains(Map map, MapKeyElement element)
{
    if(map == NULL || element == NULL) {
        return false;
    }
    MapNode check = map->head;
    while(check != NULL)
    {
        if(map->MapFunc.CompareKeyElement(check->keyElement, element) == EQUAL) {
            return true;
        }
        check = check->next;
    }
    return false;
}
MapResult mapPut(Map map, MapKeyElement keyElement, MapDataElement dataElement)
{
    if(map == NULL || keyElement == NULL || dataElement == NULL) {
        return MAP_NULL_ARGUMENT;
    }
    if(mapContains(map, keyElement) == false)
    {
        if(addNewKey(map, keyElement, dataElement) == MAP_OUT_OF_MEMORY) {
            return MAP_OUT_OF_MEMORY;
        }
        return MAP_SUCCESS;
    }
    if(updateMapKey(map, keyElement, dataElement) == MAP_OUT_OF_MEMORY){
        return MAP_OUT_OF_MEMORY;
    }
    return MAP_SUCCESS;

}

MapDataElement mapGet(Map map, MapKeyElement keyElement)
{
    if(map == NULL || keyElement == NULL) {
        return NULL;
    }
    MapNode check = map->head;
    while(check != NULL)
    {
        if(map->MapFunc.CompareKeyElement(check->keyElement,keyElement) == EQUAL)
        {
            return check->DataElement;
        }
        check = check->next;
    }
    return NULL;
}
MapResult mapRemove(Map map, MapKeyElement keyElement)
{
    if(map == NULL || keyElement == NULL) {
        return MAP_NULL_ARGUMENT;
    }
    if(mapContains(map,keyElement) == false){
        return MAP_ITEM_DOES_NOT_EXIST;
    }
    if(map->MapFunc.CompareKeyElement(map->head->keyElement,keyElement) == EQUAL)
    {
        MapNode helper = map->head;
        map->head = map->head->next;
        freeMapNode(map, helper);
        return MAP_SUCCESS;

    }
    MapNode current = map->head->next;
    MapNode tail = map->head;
    while(current != NULL)
    {
        if(map->MapFunc.CompareKeyElement(current->keyElement, keyElement) == EQUAL)
        {
            tail->next = current->next;
            freeMapNode(map, current);
            return MAP_SUCCESS;
        }
        current = current->next;
        tail = tail->next;
    }
    assert(current != NULL); //not get here
    return MAP_SUCCESS;
}

MapKeyElement mapGetFirst(Map map)
{
    if(map == NULL || mapGetSize(map) == 0) {
        return NULL;
    }
    assert(map->head != NULL);
    map->iterator = map->head;
    MapKeyElement first = map->MapFunc.CopyKeyElement(map->head->keyElement);
    return first;
}
MapKeyElement mapGetNext(Map map)
{
    if(map == NULL || map->iterator == NULL || map->iterator->next == NULL) {
        return NULL;
    }
    map->iterator = map->iterator->next;
    MapKeyElement next = map->MapFunc.CopyKeyElement(map->iterator->keyElement);
    return next;
}
MapResult mapClear(Map map)
{
    if(map == NULL) {
        return MAP_NULL_ARGUMENT;
    }
    MapNode helper = map->head;
    while(helper != NULL)
    {
        MapNode to_delete = helper;
        helper = helper->next;
        freeMapNode(map, to_delete);
    }
    map->head = NULL;
    map->iterator = NULL;
    return MAP_SUCCESS;
}
// static function:


static MapResult addNewKey(Map map, MapKeyElement keyElement, MapDataElement dataElement)
{
    assert(map != NULL && keyElement != NULL && dataElement != NULL);
    MapNode new = (MapNode)malloc(sizeof(*new));
    if(new == NULL) {
        return MAP_OUT_OF_MEMORY;
    }
    new->DataElement = map->MapFunc.CopyDataElement(dataElement);
    new->keyElement = map->MapFunc.CopyKeyElement(keyElement);
    new->next = NULL;
    if(new->DataElement == NULL || new->keyElement == NULL){
        freeMapNode(map, new);
        return MAP_OUT_OF_MEMORY;
    }
    if(map->head == NULL){
        map->head = new;
        return MAP_SUCCESS;
    }
    if (map->MapFunc.CompareKeyElement(map->head->keyElement, keyElement) > 0) {
        new->next = map->head;
        map->head = new;
        return MAP_SUCCESS;
    }
    MapNode helper = map->head->next;
    MapNode tail = map->head;
    while(helper != NULL)
    {
        if (map->MapFunc.CompareKeyElement(helper->keyElement, keyElement) > 0)
        {
            tail->next = new;
            new->next = helper;
            return MAP_SUCCESS;
        }
        helper = helper->next;
        tail = tail->next;
    }
    tail->next = new;
    return MAP_SUCCESS;
}
static MapResult updateMapKey(Map map, MapKeyElement keyElement, MapDataElement dataElement)
{
    assert(map != NULL && keyElement != NULL && dataElement != NULL);
    MapNode check = map->head;
    while (check!= NULL)
    {
        if(map->MapFunc.CompareKeyElement(check->keyElement, keyElement) == EQUAL)
        {
            MapKeyElement temp = map->MapFunc.CopyDataElement(dataElement);
            if(temp == NULL){
                return MAP_OUT_OF_MEMORY;
            }
            map->MapFunc.FreeMapDataElement(check->DataElement);
            check->DataElement = temp;
            return MAP_SUCCESS;
        }
        check = check->next;
    }
    assert(check != NULL); //not getting here
    return MAP_SUCCESS;

}
static bool checkAllocationFailed(Map copied_map, MapNode new)
{
    if(new == NULL){
        mapDestroy(copied_map);
        return true;
    }
    return false;

}
static void updateMapNode(Map map, MapNode new, MapNode original)
{
    new->keyElement = map->MapFunc.CopyKeyElement(original->keyElement);
    new->DataElement = map->MapFunc.CopyDataElement(original->DataElement);
    new->next = NULL;
}
static void freeMapNode(Map map, MapNode to_delete)
{
    if(to_delete->DataElement != NULL) {
        map->MapFunc.FreeMapDataElement(to_delete->DataElement);
    }
    if(to_delete->keyElement != NULL) {
        map->MapFunc.FreeMapKeyElement(to_delete->keyElement);
    }
    free(to_delete);
}
