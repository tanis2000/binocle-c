#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <stdbool.h>

#include "cLDtk.h"

//------------------------------------------------
//          How to compile for GCC
//------------------------------------------------
//      $> gcc -c cLDtk.c parson.c -std=c99
//      $> ar rvs libcLDtk.a cLDtk.o parson.o
//      $> gcc example.c -o example.exe libcLDtk.a -std=c99
//
//

JSON_Value *schema = NULL;
JSON_Value *user_data = NULL;

JSON_Array *tilesets_array;

JSON_Array *enums_array = NULL;
JSON_Array *enums_values = NULL;
JSON_Array *enums_externalEnums = NULL;
JSON_Array *enums_tileSrcRec = NULL;

JSON_Array *levels_layerInstances_intGrid = NULL;
JSON_Array *levels_layerInstances_intGrid_results = NULL;

JSON_Array *levels_layerInstances_autoTiles = NULL;
JSON_Array *levels_layerInstances_autoTiles_results = NULL;
JSON_Array *levels_layerInstances_autoTiles_results_tiles = NULL;

JSON_Array *levels_layerInstances_entityInstances_fieldInstances = NULL;

JSON_Array *levels_layerInstances_entityInstances_fieldInstances_value = NULL;
JSON_Array *levels_layerInstances_entityInstances_fieldInstances_realEditorValues = NULL;
JSON_Array *levels_layerInstances_entityInstances_fieldInstances_realEditorValues_params = NULL;

JSON_Array *levels_layerInstances_entityInstances = NULL;

JSON_Array *levels_layerInstances = NULL;
JSON_Array *levels_layerInstances_gridTiles = NULL;
JSON_Array *levels_layerInstances_gridTiles_results = NULL;
//JSON_Array *levels_layerInstances_gridTiles_results_tiles = NULL;

JSON_Object *levels_layerInstances_entityInstances_fieldInstances_object = NULL;
struct file_detail *levels_data_ptr = NULL;
struct levels *enums_data_ptr = NULL;
struct levels *tilesets_data_ptr = NULL;
struct levels levels;
struct file_detail file_detail;
struct file_detail *file_details_ptr = NULL;
JSON_Array *levels_neighbors = NULL;

JSON_Array *levels_array = NULL;




//import file specification data
void importFileSpecs(void){
    file_details_ptr = &file_detail;
     //File spec data
    file_details_ptr->jsonVersion = json_object_get_string(json_object(user_data), "jsonVersion");
    file_details_ptr->defaultPivotX = json_object_get_number(json_object(user_data), "defaultPivotX");
    file_details_ptr->defaultPivotY = json_object_get_number(json_object(user_data), "defaultPivotY");
    file_details_ptr->defaultGridSize = json_object_get_number(json_object(user_data), "defaultGridSize");
    file_details_ptr->bgColor = json_object_get_string(json_object(user_data), "bgColor");
    file_details_ptr->nextUid = json_object_get_number(json_object(user_data), "nextUid");
}

//import tileset data
void importTilesetData(void){

    //def.tilesets data
    tilesets_array = json_object_dotget_array(json_object(user_data), "defs.tilesets");

    levels.tilesets_data_ptr = malloc(sizeof(struct tilesets) * json_array_get_count(tilesets_array));
    tilesets_data_ptr = &levels;



    for(int i=0;i<json_array_get_count(tilesets_array);i++){


        tilesets_data_ptr->tilesets_data_ptr[i].identifier =json_object_get_string( json_array_get_object(tilesets_array, i), "identifier");
        tilesets_data_ptr->tilesets_data_ptr[i].uid =json_object_get_number( json_array_get_object(tilesets_array, i), "uid");
        tilesets_data_ptr->tilesets_data_ptr[i].relPath =json_object_get_string( json_array_get_object(tilesets_array, i), "relPath");
        tilesets_data_ptr->tilesets_data_ptr[i].pxWid =json_object_get_number( json_array_get_object(tilesets_array, i), "pxWid");
        tilesets_data_ptr->tilesets_data_ptr[i].pxHei =json_object_get_number( json_array_get_object(tilesets_array, i), "pxHei");
        tilesets_data_ptr->tilesets_data_ptr[i].tileGridSize =json_object_get_number( json_array_get_object(tilesets_array, i), "tileGridSize");
        tilesets_data_ptr->tilesets_data_ptr[i].spacing =json_object_get_number( json_array_get_object(tilesets_array, i), "spacing");
        tilesets_data_ptr->tilesets_data_ptr[i].padding =json_object_get_number( json_array_get_object(tilesets_array, i), "padding");

    }
}

//free tileset data from memory
void freeTilesetData(void){

    free(tilesets_data_ptr->tilesets_data_ptr);

}


//import enums data
void importEnumsData(void){

    //defs.enums data
    enums_array = json_object_dotget_array(json_object(user_data), "defs.enums");


    levels.enums_data_ptr = malloc(sizeof(struct enums) * json_array_get_count(enums_array));
    enums_data_ptr = &levels;


    for(int i=0;i<json_array_get_count(enums_array);i++){

        enums_data_ptr->enums_data_ptr[i].identifier =json_object_get_string( json_array_get_object(enums_array, i), "identifier");
        enums_data_ptr->enums_data_ptr[i].uid =json_object_get_number( json_array_get_object(enums_array, i), "uid");

        //defs.enums_array.values
        //Get array inside of array
        enums_values =json_object_get_array( json_array_get_object(enums_array, i), "values");

        enums_data_ptr->enums_data_ptr[i].enums_details_ptr = malloc(sizeof(struct enums_details) * json_array_get_count(enums_values));

        for(int g=0;g<json_array_get_count(enums_values);g++){
            //Get values from array
            enums_data_ptr->enums_data_ptr[i].enums_details_ptr[g].id =json_object_get_string( json_array_get_object(enums_values, g), "id");
            enums_data_ptr->enums_data_ptr[i].enums_details_ptr[g].tileId =json_object_get_number( json_array_get_object(enums_values, g), "tileId");
            enums_tileSrcRec =json_object_get_array( json_array_get_object(enums_values, g), "__tileSrcRect");


                enums_data_ptr->enums_data_ptr[i].enums_details_ptr[g].tileSrcRec[0] =  json_array_get_number(enums_tileSrcRec,0);
                enums_data_ptr->enums_data_ptr[i].enums_details_ptr[g].tileSrcRec[1] = json_array_get_number(enums_tileSrcRec,1);
                enums_data_ptr->enums_data_ptr[i].enums_details_ptr[g].tileSrcRec[2] = json_array_get_number(enums_tileSrcRec,2);
                enums_data_ptr->enums_data_ptr[i].enums_details_ptr[g].tileSrcRec[3] = json_array_get_number(enums_tileSrcRec,3);


        }

        enums_data_ptr->enums_data_ptr[i].iconTilesetUid =json_object_get_number( json_array_get_object(enums_array, i), "iconTilesetUid");

    }
}

//free enums data from memory
void freeEnumsData(void){

    //defs.enums data
    enums_array = json_object_dotget_array(json_object(user_data), "defs.enums");


    for(int i=0;i<json_array_get_count(enums_array);i++){

        free(enums_data_ptr->enums_data_ptr[i].enums_details_ptr);

    }

    free(enums_data_ptr->enums_data_ptr);

}


//import level data
void importLevelsData(void){

    //defs.levels
    levels_array = json_object_get_array(json_object(user_data), "levels");


    file_detail.levels_data_ptr = malloc(sizeof(struct levels) * json_array_get_count(levels_array));
    levels_data_ptr = &file_detail;

    for(int i=0;i<json_array_get_count(levels_array);i++){


        levels_data_ptr->levels_data_ptr[i].identifier =json_object_get_string( json_array_get_object(levels_array, i), "identifier");

        levels_data_ptr->levels_data_ptr[i].uid =json_object_get_number(json_array_get_object(levels_array, i), "uid"); //i; //used to reference index for each level
        levels_data_ptr->levels_data_ptr[i].pxWid =json_object_get_number( json_array_get_object(levels_array, i), "pxWid");
        levels_data_ptr->levels_data_ptr[i].pxHei =json_object_get_number( json_array_get_object(levels_array, i), "pxHei");
        levels_data_ptr->levels_data_ptr[i].worldX =json_object_get_number( json_array_get_object(levels_array, i), "worldX");
        levels_data_ptr->levels_data_ptr[i].worldY =json_object_get_number( json_array_get_object(levels_array, i), "worldY");

        JSON_Array *level_field_instance_array = json_object_get_array(json_array_get_object(levels_array, i), "fieldInstances");
        JSON_Object *first_instance = json_array_get_object(level_field_instance_array, 0);
        if (first_instance) {
            levels_data_ptr->levels_data_ptr[i].firstIntFieldInst = (int)json_object_get_number(first_instance, "__value");
        }

        //__neighbors
        //levels.neighbors
        levels_neighbors =json_object_get_array(json_array_get_object(levels_array, i), "__neighbours");
        levels_data_ptr->levels_data_ptr[i].numNeighbors = json_array_get_count(levels_neighbors);
        levels_data_ptr->levels_data_ptr[i].neighbors = calloc(levels_data_ptr->levels_data_ptr[i].numNeighbors, sizeof(struct levelNeighbors));
        for (int g=0;g<levels_data_ptr->levels_data_ptr[i].numNeighbors;g++){
            levels_data_ptr->levels_data_ptr[i].neighbors[g].dir = json_object_get_string(json_array_get_object(levels_neighbors, g), "dir")[0];
            levels_data_ptr->levels_data_ptr[i].neighbors[g].uid = (int)json_object_get_number(json_array_get_object(levels_neighbors, g), "levelUid");
        }

        //layerInstances
        //levels.layerInstances
        //Get array inside of array
        levels_layerInstances =json_object_get_array( json_array_get_object(levels_array, i), "layerInstances");

        levels_data_ptr->levels_data_ptr[i].layers_data_ptr = malloc(sizeof(struct layerInstances) * json_array_get_count(levels_layerInstances));

        for(int g=0;g<json_array_get_count(levels_layerInstances);g++){
            //Get values from array
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].identifier =json_object_get_string( json_array_get_object(levels_layerInstances, g), "__identifier");
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].type =json_object_get_string( json_array_get_object(levels_layerInstances, g), "__type");
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].cWid =json_object_get_number( json_array_get_object(levels_layerInstances, g), "__cWid");
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].cHei =json_object_get_number( json_array_get_object(levels_layerInstances, g), "__cHei");
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridSize =json_object_get_number( json_array_get_object(levels_layerInstances, g), "__gridSize");
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].levelId =json_object_get_number( json_array_get_object(levels_layerInstances, g), "levelId");
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].layerDefUid =json_object_get_number( json_array_get_object(levels_layerInstances, g), "layerDefUid");
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].pxOffsetX =json_object_get_number( json_array_get_object(levels_layerInstances, g), "pxOffsetX");
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].pxOffsetY =json_object_get_number( json_array_get_object(levels_layerInstances, g), "pxOffsetY");


            //// BEGIN OF ENTITIES
            if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].type,"Entities") == 0){

                //entityInstances
                //levels.layerInstances.entityInstances
                //Get array inside of array

                levels_layerInstances_entityInstances =json_object_get_array( json_array_get_object(levels_layerInstances, g), "entityInstances");


                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr = malloc(sizeof(struct entityInstances) * json_array_get_count(levels_layerInstances_entityInstances));
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].numEntityInstancesDataPtr = json_array_get_count(levels_layerInstances_entityInstances);

                for(int y=0;y<json_array_get_count(levels_layerInstances_entityInstances);y++){

                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].identifier =json_object_get_string( json_array_get_object(levels_layerInstances_entityInstances, y), "__identifier");


                    levels_layerInstances_autoTiles_results =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances, y), "__grid");
                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].cx =  json_array_get_number(levels_layerInstances_autoTiles_results,0);

                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].cy = json_array_get_number(levels_layerInstances_autoTiles_results,1);


                    levels_layerInstances_autoTiles_results =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances, y), "px");
                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].x =json_array_get_number(levels_layerInstances_autoTiles_results,0);

                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].y =json_array_get_number(levels_layerInstances_autoTiles_results,1);


                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].defUid =json_object_get_number( json_array_get_object(levels_layerInstances_entityInstances, y), "defUid");



                    //fieldInstances
                    //levels.layerInstances.entityInstances.fieldInstances
                    //Get array inside of array
                    levels_layerInstances_entityInstances_fieldInstances =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances, y), "fieldInstances");


                    if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances) != 0){

                        levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstanceCount = json_array_get_count(levels_layerInstances_entityInstances_fieldInstances);
                    }


                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr = malloc(sizeof(struct fieldInstances) * json_array_get_count(levels_layerInstances_entityInstances_fieldInstances));


                    for(int p=0;p<json_array_get_count(levels_layerInstances_entityInstances_fieldInstances);p++){


                        levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].identifier =json_object_get_string( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__identifier");


                        //Get type for item value if it is an array type
                        levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type =json_object_get_string( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__type");


                        //
                        //  ***   Need to check each type and allocate the fieldInstances_ accordingly   ***
                        //

                        //
                        //fieldInstances_points
                        //


                        if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_point) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){
                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr = malloc(sizeof(struct fieldInstances_points) * json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value));



                                for(int k=0;k<json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);k++){

                                    //multiply with gridsize to get real screen cordinates
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr[k].cx = levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridSize * json_object_get_number( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances_value, k), "cx");

                                    //multiply with gridsize to get real screen cordinates
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr[k].cy = levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridSize * json_object_get_number( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances_value, k), "cy");
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr[k].check = 1;
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr[0].size = k+1;

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);;


                                }

                            }
                            else{

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size =0;

                            }
                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_point) == 0){


                            if(json_object_get_object( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value") == NULL){
                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = 0;

                            }
                            else{
                                levels_layerInstances_entityInstances_fieldInstances_object =json_object_get_object( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr = malloc(sizeof(struct fieldInstances_points) );


                                //multiply with gridsize to get real screen cordinates
                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr[0].cx = levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridSize * json_object_get_number( levels_layerInstances_entityInstances_fieldInstances_object, "cx");

                                //multiply with gridsize to get real screen cordinates
                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr[0].cy = levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridSize * json_object_get_number( levels_layerInstances_entityInstances_fieldInstances_object, "cy");
                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr[0].check = 1;
                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr[0].size = 1;
                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = 1;
                            }

                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_string) == 0 ||
                                strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_color) == 0 ||
                                strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_filePath) == 0 ||
                                strncmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_localEnum,9) == 0 ){
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr = malloc(sizeof(struct fieldInstances_strings));

                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr->strValue =json_object_get_string( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr->check = 1;
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr->size = 1;
							levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = 1;
                        }
                        if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_int) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){
                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr = malloc(sizeof(struct fieldInstances_ints) * json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value));



                                for(int k=0;k<json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);k++){

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr[k].intValue =  json_array_get_number(levels_layerInstances_entityInstances_fieldInstances_value, k);
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr[k].check = 1;
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr[0].size = k+1;

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);;


                                }
                            }
                            else{
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size =0;

                            }
                        }
                        if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_float) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){

                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr = malloc(sizeof(struct fieldInstances_floats) * json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value));



                                for(int k=0;k<json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);k++){

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr[k].floatValue =  json_array_get_number(levels_layerInstances_entityInstances_fieldInstances_value, k);
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr[k].check = 1;
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr[0].size = k+1;

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);;


                                }

                            }
                            else{

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size =0;

                            }
                        }
                        if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_boolean) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){
                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr = malloc(sizeof(struct fieldInstances_ints) * json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value));

                                for(int k=0;k<json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);k++){

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr[k].intValue =  json_array_get_boolean(levels_layerInstances_entityInstances_fieldInstances_value, k);
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr[k].check = 1;
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr[0].size = k+1;

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);;

                                }
                            }
                            else{

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size =0;

                            }
                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_integer) == 0 ){
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr = malloc(sizeof(struct fieldInstances_ints));

                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr->intValue =json_object_get_number( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr->check = 1;
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr->size = 1;
							levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = 1;
                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_boolean) == 0 ){
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr = malloc(sizeof(struct fieldInstances_ints));

                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr->intValue =json_object_get_boolean( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr->check = 1;
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr->size = 1;
							levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = 1;
                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_float) == 0 ){
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr = malloc(sizeof(struct fieldInstances_floats));

                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr->floatValue =json_object_get_number( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr->check = 1;
                            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr->size = 1;
							levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = 1;
                        }
                        else if(strncmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_localEnum,15) == 0 ||
                        strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_filePath) == 0 ||
                        strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_string) == 0 ||
                        strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_color) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){

                                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr = malloc(sizeof(struct fieldInstances_strings) * json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value));

                                for(int k=0;k<json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);k++){

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr[k].strValue = json_array_get_string(levels_layerInstances_entityInstances_fieldInstances_value, k);
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr[k].check = 1;
                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr[0].size = k+1;

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size  = json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value);;




                                }

                            }
                            else{

                                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].size =0;

                            }
                        }






                    }


                }
            }//// END OF ENTITIES


            //////////
            //IntGridCsv
            //levels.layerInstances.intGrid/intGridWidth/intGridHeight
            //Get the int grid
            JSON_Array *intGrid = json_object_get_array( json_array_get_object(levels_layerInstances, g), "intGridCsv");
            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].intGrid = malloc(sizeof(int) *
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].cWid *
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].cHei
            );
            for (int y = 0; y < json_array_get_count(intGrid); y++) {
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].intGrid[y] = (int)json_array_get_number(intGrid, y);
            }


            //////////
            //AutoTiles
            //levels.layerInstances.autoTiles
            //Get array inside of array
            levels_layerInstances_autoTiles =json_object_get_array( json_array_get_object(levels_layerInstances, g), "autoLayerTiles");


            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].autoTiles_data_ptr = malloc(sizeof(struct autoTiles) * json_array_get_count(levels_layerInstances_autoTiles));


            for(int y=0;y<json_array_get_count(levels_layerInstances_autoTiles);y++){

                levels_layerInstances_autoTiles_results =json_object_get_array( json_array_get_object(levels_layerInstances_autoTiles, y), "px");


                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].autoTiles_data_ptr[y].x = json_array_get_number(levels_layerInstances_autoTiles_results,0);
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].autoTiles_data_ptr[y].y =json_array_get_number(levels_layerInstances_autoTiles_results,1);

                levels_layerInstances_autoTiles_results =json_object_get_array( json_array_get_object(levels_layerInstances_autoTiles, y), "src");

                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].autoTiles_data_ptr[y].SRCx = json_array_get_number(levels_layerInstances_autoTiles_results,0);
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].autoTiles_data_ptr[y].SRCy =json_array_get_number(levels_layerInstances_autoTiles_results,1);
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].autoTiles_data_ptr[y].f =json_object_get_number( json_array_get_object(levels_layerInstances_autoTiles, y), "f");
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].autoTiles_data_ptr[y].t =json_object_get_number( json_array_get_object(levels_layerInstances_autoTiles, y), "t");

                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].autoTiles_data_ptr->count = y+1;


            }
            //////////

            //////////
            //GridTiles
            //levels.layerInstances.gridTiles
            //Get array inside of array
            levels_layerInstances_gridTiles =json_object_get_array( json_array_get_object(levels_layerInstances, g), "gridTiles");


            levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridTiles_data_ptr = malloc(sizeof(struct gridTiles) * json_array_get_count(levels_layerInstances_gridTiles));


            for(int y=0;y<json_array_get_count(levels_layerInstances_gridTiles);y++){

                levels_layerInstances_gridTiles_results =json_object_get_array( json_array_get_object(levels_layerInstances_gridTiles, y), "px");


                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridTiles_data_ptr[y].x = json_array_get_number(levels_layerInstances_gridTiles_results,0);
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridTiles_data_ptr[y].y =json_array_get_number(levels_layerInstances_gridTiles_results,1);

                levels_layerInstances_gridTiles_results =json_object_get_array( json_array_get_object(levels_layerInstances_gridTiles, y), "src");

                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridTiles_data_ptr[y].SRCx = json_array_get_number(levels_layerInstances_gridTiles_results,0);
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridTiles_data_ptr[y].SRCy =json_array_get_number(levels_layerInstances_gridTiles_results,1);
                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridTiles_data_ptr[y].f =json_object_get_number( json_array_get_object(levels_layerInstances_gridTiles, y), "f");

                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridTiles_data_ptr->count = y+1;


            }
            //////////

            //////////
            //IntGrid
            //levels.layerInstances.intGrid
            //Get array inside of array
            if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].type,"IntGrid") == 0){
                levels_layerInstances_intGrid =json_object_get_array( json_array_get_object(levels_layerInstances, g), "intGrid");


                levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].integerGrid_data_ptr = malloc(sizeof(struct integerGrid) * json_array_get_count(levels_layerInstances_intGrid));

                for(int y=0;y<json_array_get_count(levels_layerInstances_intGrid);y++){

                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].integerGrid_data_ptr[y].coordId = (int)json_object_get_number( json_array_get_object(levels_layerInstances_intGrid, y), "coordId");
                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].integerGrid_data_ptr[y].v = (int)json_object_get_number( json_array_get_object(levels_layerInstances_intGrid, y), "v");
                    levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].integerGrid_data_ptr->count = y+1;

                }
            }
            //////////






        }

    }

}//End of importLevelsData


//free level data from memory
void freeLevelsData(void){


    //defs.levels
    levels_array = json_object_get_array(json_object(user_data), "levels");



    for(int i=0;i<json_array_get_count(levels_array);i++){

        //level neighbors
        free(levels_data_ptr->levels_data_ptr[i].neighbors);

        //layerInstances
        //levels.layerInstances
        //Get array inside of array
        levels_layerInstances =json_object_get_array( json_array_get_object(levels_array, i), "layerInstances");



        for(int g=0;g<json_array_get_count(levels_layerInstances);g++){



            //// BEGIN OF ENTITIES
            if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].type,"Entities") == 0){

                //entityInstances
                //levels.layerInstances.entityInstances
                //Get array inside of array

                levels_layerInstances_entityInstances =json_object_get_array( json_array_get_object(levels_layerInstances, g), "entityInstances");


                for(int y=0;y<json_array_get_count(levels_layerInstances_entityInstances);y++){


                    //fieldInstances
                    //levels.layerInstances.entityInstances.fieldInstances
                    //Get array inside of array
                    levels_layerInstances_entityInstances_fieldInstances =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances, y), "fieldInstances");


                    for(int p=0;p<json_array_get_count(levels_layerInstances_entityInstances_fieldInstances);p++){


                        if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_point) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){

                                free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr);

                            }

                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_point) == 0){


                            if(json_object_get_object( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value") == NULL){


                            }
                            else{


                                free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_points_ptr);
                            }


                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_string) == 0 ||
                                strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_color) == 0 ||
                                strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_filePath) == 0 ||
                                strncmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_localEnum,9) == 0 ){
                            free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr);
                        }
                        if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_int) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){

                                free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr);

                            }

                        }
                        if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_float) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){
                                free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr);
                            }

                        }
                        if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_boolean) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){
                              free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr);
                            }

                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_integer) == 0 ){
                            free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr);
                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_boolean) == 0 ){
                            free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_ints_ptr);
                        }
                        else if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_float) == 0 ){
                            free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_floats_ptr);
                        }
                        else if(strncmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_localEnum,15) == 0 ||
                        strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_filePath) == 0 ||
                        strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_string) == 0 ||
                        strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].type,fieldInstance_array_color) == 0){

                            levels_layerInstances_entityInstances_fieldInstances_value =json_object_get_array( json_array_get_object(levels_layerInstances_entityInstances_fieldInstances, p), "__value");

                            //Check for empty arrays
                            if(json_array_get_count(levels_layerInstances_entityInstances_fieldInstances_value) != 0){
                                free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr[p].fieldInstances_strings_ptr);
                            }

                        }






                    }
                    free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr[y].fieldInstances_data_ptr);


                }
                free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].entityInstances_data_ptr);
            }//// END OF ENTITIES


            //////////
            //IntGridCsv
            //levels.layerInstances.intGrid
            //Free array
            free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].intGrid);

            //////////
            //AutoTiles
            //levels.layerInstances.autoTiles
            //Get array inside of array

            free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].autoTiles_data_ptr);

            //////////

            //////////
            //GridTiles
            //levels.layerInstances.gridTiles
            //Get array inside of array

            free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].gridTiles_data_ptr);

            //////////

            //////////
            //IntGrid
            //levels.layerInstances.intGrid
            //Get array inside of array
            if(strcmp(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].type,"IntGrid") == 0){
                free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr[g].integerGrid_data_ptr);
            }
            //////////








        }
        free(levels_data_ptr->levels_data_ptr[i].layers_data_ptr);

    }
    free(file_detail.levels_data_ptr);
}


//return level data as struct
struct levels* ldtk_get_level(char* levelName){
    struct levels *ptr_le;
    ptr_le = NULL;

    levels_array = json_object_get_array(json_object(user_data), "levels");
    for(int i=0;i<json_array_get_count(levels_array);i++){

        if(strcmp(levels_data_ptr->levels_data_ptr[i].identifier,levelName)==0){

            ptr_le = &levels_data_ptr->levels_data_ptr[i];

        }

    }
    return(ptr_le);
}

int getIdFromUid(int levelUId){
    int id = 0;

    levels_array = json_object_get_array(json_object(user_data), "levels");
    for(int i=0;i<json_array_get_count(levels_array);i++){

        if(levels_data_ptr->levels_data_ptr[i].uid == levelUId){

            id = i;
            break;

        }

    }
    return id;
}

struct levels* getLevelFromUid(int levelId){
    struct levels *ptr_le;
    ptr_le = NULL;

    levels_array = json_object_get_array(json_object(user_data), "levels");
    for(int i=0;i<json_array_get_count(levels_array);i++){

        if(levels_data_ptr->levels_data_ptr[i].uid == levelId){

            ptr_le = &levels_data_ptr->levels_data_ptr[i];

        }

    }
    return(ptr_le);
}

//return entity as struct
struct entityInstances* getEntity(char* entityName,int levelUId){
    int levelId = getIdFromUid(levelUId);

    struct entityInstances* ptr_ed;
    int ptr_buffer = 2;
    ptr_ed = malloc(ptr_buffer * sizeof(struct entityInstances));
    int arrayCounter = 0;


        levels_layerInstances =json_object_get_array( json_array_get_object(levels_array, levelId), "layerInstances");
        for(int g=0;g<json_array_get_count(levels_layerInstances);g++){

            //// BEGIN OF ENTITIES
            if(strcmp(levels_data_ptr->levels_data_ptr[levelId].layers_data_ptr[g].type,"Entities") == 0){

                levels_layerInstances_entityInstances =json_object_get_array( json_array_get_object(levels_layerInstances, g), "entityInstances");

                for(int y=0;y<json_array_get_count(levels_layerInstances_entityInstances);y++){

                    //Reallocate if more space is needed
                    if(arrayCounter >= ptr_buffer){
                        ptr_buffer *= ptr_buffer;
                        ptr_ed = realloc(ptr_ed,ptr_buffer * sizeof(struct entityInstances));

                    }


                    if(strcmp(levels_data_ptr->levels_data_ptr[levelId].layers_data_ptr[g].entityInstances_data_ptr[y].identifier,entityName)==0){

                        ptr_ed[arrayCounter] = levels_data_ptr->levels_data_ptr[levelId].layers_data_ptr[g].entityInstances_data_ptr[y];

                        arrayCounter++;

                        //store the size in the first element
                        ptr_ed[0].size = arrayCounter;


                    }

                }

            }//// END OF ENTITIES
        }

    return(ptr_ed);
}


//return layer as struct
struct layerInstances* getLayer(char* layerName,int levelUId){
    int levelId = getIdFromUid(levelUId);

    struct layerInstances *ptr_li;
    ptr_li = NULL;

    levels_array = json_object_get_array(json_object(user_data), "levels");
    for(int i=0;i<json_array_get_count(levels_array);i++){

        levels_layerInstances =json_object_get_array( json_array_get_object(levels_array, i), "layerInstances");
        for(int g=0;g<json_array_get_count(levels_layerInstances);g++){


            if(strcmp(levels_data_ptr->levels_data_ptr[levelId].layers_data_ptr[g].identifier,layerName)==0){

                ptr_li = &levels_data_ptr->levels_data_ptr[levelId].layers_data_ptr[g];
            }
        }
    }
    return(ptr_li);
}


//load JSON file into memory
void ldtk_load_json_file(char* fileName) {
    user_data = json_parse_file(fileName);
    if (user_data == NULL) {
        user_data = json_value_init_object();
    }
}

void ldtk_load_json_from_memory(const char *stream) {
  user_data = json_parse_string(stream);
  if (user_data == NULL) {
    user_data = json_value_init_object();
  }
}

//load map data from JSON file into memory
void ldtk_import_map_data(void){
    importFileSpecs();
    importTilesetData();
    importEnumsData();
    importLevelsData();
}

//free map data from memory
void freeMapData(void){
    freeTilesetData();
    freeEnumsData();
    freeLevelsData();
}
