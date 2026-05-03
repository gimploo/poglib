


-- ECS -- (for 3d games)


entity 
-> transform
-> collider
-> render

Q. How to map index here to hashtable ?
   -  We could have each entity_id map to a master array that holds all the entities together

entity_t **entity_list - [ e1, e2, e3, e6, e5 ...] (*MASTER*) 
    - this would hold the actual entity itself and not just the entity_id
    - u32 *transform   - [ e1, e2, e3, e6, e5 ...]
      u32 *render      - [ _ , _ , e3, _ , _ ... ]
    - 

ecs {

    1. Hashtable(entity_id, array_index in entitylist) - DONE

    2. entitylist -> what does this mean - do we include index of entities or objects themselves
        - not objects
        - master list

    3. Better to include


}
