#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <math.h>


#define INIT "init"
#define CHANGE_COST "change_cost"
#define TOGGLE_AIR_ROUTE "toggle_air_route"
#define TRAVEL_COST "travel_cost"
#define MAX_AIR_ROUTE 5
#define START_COST 1
#define MAX_ADJACENT_CELL 6
#define CACHE 1000


typedef struct coord
{
    int x;
    int y;
} coordinates; 


typedef struct route_
{
    coordinates coord; 
    int cost;
    struct route_ * next;
} route_node; 

typedef struct cac_ {
    int x1; int y1;
    int x2; int y2;
    int costo;
    struct cac_ * next;

} cache;

cache * cache_array[CACHE] = {NULL};

typedef struct hex
{
    int cost; 
    route_node *routes;
} hexagon; 

typedef struct {
    coordinates coord;
    int cost_from_start;
} node;

typedef struct dijkstra_helper_{
    node *q;
    int head_q;
    int dim_q;
} min_heap;

int distHexagons(int x1, int y1, int x2, int y2);
int max_int(int, int);
int min_int(int, int);
float max_float(float, float);
float min_float(float, float);
int apply_cost_bound(int c);
void toggle_route(int, int, int x, int y, int , int);

/*function for min-heap handdling*/
void min_heapify(node* nodes, int dim, int index);
int left(int i);
int right(int i);
int parent(int i);
node heap_extract_min(node *nodes, int *dim);
void neighboord_calculate(int x, int y);
void insert_heap(min_heap *, int new_cost, int x_vic, int y_vic);
int hash(int x1, int y1, int x2, int y2) {

    int incre = (x1 + y2 * 3 + (17) * y1 + y2) % CACHE;
    int incr2 = (y2 + x2 * 7 + 19 * x1 + x2) % CACHE;
    int val = incre + incr2;
    

    return val % CACHE;
}


int trova_cache(int x1, int y1, int x2, int y2) {
    int index = hash(x1, y1, x2, y2);
    cache * curr = cache_array[index];

    while(curr) {
        if(curr->x1 == x1 && curr->y1 == y1 && curr->x2 == x2 && curr->y2 == y2) {
            return curr->costo;
        }
        curr = curr->next;
    }
    return INT_MIN;
}

void put_cache(int x1, int y1, int x2, int y2, int costo) {
    int index = hash(x1, y1, x2, y2);

    cache * new = malloc(sizeof(cache));

    new->x1 = x1;
    new->y1 = y1;
    new->x2 = x2;
    new->y2 = y2;
    new->costo = costo;

    new->next = cache_array[index];
    cache_array[index] = new;
}

void libera_cache() {
    
    for(int i = 0; i < CACHE; i++) {
        cache * ptr = cache_array[i];

        while(ptr) {
            cache * succ = ptr->next;
            free(ptr);
            ptr = succ;
        }

        cache_array[i] = NULL;
    }
}

coordinates frontier_g[MAX_ADJACENT_CELL];

FILE *input, *output;


hexagon **map = NULL;

int main(int argc, char *argv[]){   
    
    int sc;
    int dim_r;
    int dim_c;
    char command[20];
    min_heap heap;

    heap.q = NULL;
    heap.dim_q = 0;
    heap.head_q = 0;

    int **cost_table = NULL;
    input = stdin;
    output = stdout;


    while (fscanf(input, "%s", command)==1)
    {
        //printf("%d: ", num);

        if (command[1] == 'r')
        {  

            int x1, y1, x2, y2;
            sc = fscanf(input, "%d" "%d" "%d" "%d", &x1, &y1, &x2, &y2);
            if(map == NULL) {
                printf("-1\n");
                continue;
            }

            if(x1 < 0 || x1 >= dim_c || y1 < 0 || y1 >= dim_r) {
                printf("-1\n");
                continue;
            }


            int path_cost = trova_cache(x1, y1, x2, y2);
            if(path_cost != INT_MIN) {
                fprintf(output, "%d\n", path_cost);
                continue;
            }

            

            path_cost =-1;
            /*initialization min-heap and cost_table*/
            heap.q = malloc(sizeof(node) * 1000);
            heap.dim_q = 1000;
            heap.head_q = 1;

            heap.q[0].coord.x=x1;
            heap.q[0].coord.y=y1;
            heap.q[0].cost_from_start = 0;

            if(cost_table == NULL) {
                cost_table = malloc(sizeof(int *) * dim_r);
                for(int i = 0; i < dim_r; i++) {
                    cost_table[i] = malloc(sizeof(int *) * dim_r);
                    for(int j = 0; j < dim_c; j++) {
                        cost_table[i][j] = INT_MAX;
                    }
                }
            } else {
                for(int i = 0; i < dim_r; i++) {
                    for(int j = 0; j < dim_c; j++) {
                        cost_table[i][j] = INT_MAX;
                    }
                }
            }

            cost_table[y1][x1] = 0;

            
            /*end initialization */
            
            
            while(heap.head_q!=0){
                //heap_extract_min(heap.q, &heap.dim_q);
                node u = heap_extract_min(heap.q, &(heap.head_q));

                if(u.coord.x ==x2 && u.coord.y == y2){
                    path_cost = u.cost_from_start;
                    break;
                }
                if(map[u.coord.y][u.coord.x].cost == 0) continue;
                

                neighboord_calculate(u.coord.x, u.coord.y);

                for(int i = 0; i < MAX_ADJACENT_CELL; i++) {
                    if(frontier_g[i].x < 0 || frontier_g[i].x >= dim_c || 
                       frontier_g[i].y < 0 || frontier_g[i].y >= dim_r) {
                        continue;
                    } else {
                        int x_vic = frontier_g[i].x;
                        int y_vic = frontier_g[i].y;

                        // ridondante? if(x_vic < 0 || x_vic >= dim_c || y_vic < 0 || y_vic >= dim_r) continue;

                        int new_cost = u.cost_from_start + map[u.coord.y][u.coord.x].cost;

                        if(new_cost < cost_table[y_vic][x_vic]) {
                            insert_heap(&heap, new_cost, x_vic, y_vic);
                            cost_table[y_vic][x_vic] = new_cost;
                        }
                    }
                }
                
                // SCORRI SU LISTA CONCAT!!!
                // for(int i=0; i<map[u.coord.x][u.coord.y].i; i++, index_a++)
                //     frontier_a[index_a] = map[u.coord.x][u.coord.y].routes[i].coord;
                /*end of frontier construction*/
                /*relaxation*/

                route_node * list = map[u.coord.y][u.coord.x].routes;

                while(list) {

                    int new_cost = u.cost_from_start + list->cost;

                    if(new_cost < cost_table[list->coord.y][list->coord.x]) {
                        cost_table[list->coord.y][list->coord.x] = new_cost;
                        insert_heap(&heap, new_cost, list->coord.x, list->coord.y);
                    }


                    list = list->next;
                }
                
            }

            put_cache(x1, y1, x2, y2, path_cost);


            fprintf(output,"%d\n", path_cost);
            
            free(heap.q);

        }
        else if (command[1] == 'o')
        {
            
            int x1, y1, x2, y2;
            sc = fscanf(input, "%d" "%d" "%d" "%d", &x1, &y1, &x2, &y2);
            toggle_route(x1, y1, x2, y2, dim_c, dim_r);
        }
        else if (command[1] == 'h')
        {
            float rag;
            int x;
            int y;
            int v;
            sc = fscanf(input, "%d", &x);
            sc = fscanf(input, "%d", &y);
            sc = fscanf(input, "%d", &v);
            sc = fscanf(input, "%f", &rag);

            if(x < 0 || x >= dim_c || y < 0 || y >= dim_r || rag <= 0 || v < -10 || v > 10) {
                printf("KO\n");
                continue;
            }

            int q, r = 0;
            if(y % 2) {
                q = x - (y - 1) /2;
            } else {
                q = x - y / 2;
            }
            r = y;

            for (int i = -rag; i <=rag; i++){
                for (int j = -rag; j <= +rag; j++){
                    int qn = q + i;
                    int rn = r +j;

                    int xn , yn = 0;
                    if(rn % 2) {
                        xn = qn + (rn - 1) / 2;
                    } else {
                        xn = qn + rn / 2;
                    }
                    yn = rn;

                    if(yn < 0 || xn < 0 || yn >= dim_r || xn >= dim_c) {
                        continue;
                    }


                    int dist = distHexagons(x, y, xn, yn);
                    if (dist < rag){ // [-10,+10]
                        int c = floor(v * max_float(0, (rag - dist) / rag));
                        map[yn][xn].cost = apply_cost_bound(map[yn][xn].cost + c);
                        
                        route_node * lista = map[yn][xn].routes;

                        while(lista) {

                            lista->cost = apply_cost_bound(lista->cost + c);
                            lista = lista->next;
                        }
                    }

                }
            }
            libera_cache();

            printf("OK\n");
        }
        else if (command[0] == 'i')
        {
            printf("OK\n");
            libera_cache();
            if(map) {
                for(int i = 0; i < dim_r; i++) {
                    for(int j = 0; j < dim_c; j++) {
                        route_node * list = map[i][j].routes;
                        while(list) {
                            route_node *tmp = list->next;
                            free(list);
                            list = tmp;
                        }
                    }
                    free(map[i]);
                }

                free(map);
            }

            if(cost_table) {
                for(int i = 0; i < dim_r; i++) {
                    free(cost_table[i]);
                }
                free(cost_table);
            }

            sc = fscanf(input, "%d", &dim_c);
            sc = fscanf(input, "%d", &dim_r);
            map = malloc(dim_r * sizeof(hexagon *));
            for (int i = 0; i < dim_r; i++){
                map[i] = malloc(dim_c * sizeof(hexagon));
                for(int j=0; j<dim_c; j++){
                    map[i][j].cost=START_COST;
                    map[i][j].routes = NULL;
                }
            }


            cost_table = malloc(dim_r * sizeof(int *));
            for (int i = 0; i < dim_r; i++){
                cost_table[i] = malloc(dim_c * sizeof(int));
                for(int j=0; j<dim_c; j++)
                    cost_table[i][j] = INT_MAX;
            }
        } 
    }
    sc--;

    if(map) {
        for(int i = 0; i < dim_r; i++) {
            for(int j = 0; j < dim_c; j++) {
                route_node * list = map[i][j].routes;
                while(list) {
                    route_node *tmp = list->next;
                    free(list);
                    list = tmp;
                }
            }
            free(map[i]);
        }

        free(map);
    }

    if(cost_table) {
        for(int i = 0; i < dim_r; i++) {
            free(cost_table[i]);
        }
        free(cost_table);
    }

    libera_cache();


    return 0;
}

    

int distHexagons(int x1, int y1, int x2, int y2)
{

    int q1, r1 = 0;
    if(y1 % 2) {
        q1 = x1 - (y1 - 1) /2;
    } else {
        q1 = x1 - y1 / 2;
    }
    r1 = y1;

    int q2, r2 = 0;
    if(y2 % 2) {
        q2 = x2 - (y2 - 1) /2;
    } else {
        q2 = x2 - y2 / 2;
    }
    r2 = y2;


    return (abs(r1-r2) + abs(q1-q2) + abs(q1+r1-r2-q2))/ 2;
}

int max_int(int a, int b)
{
    if (a > b)
        return a;
    return b;
}

int min_int(int a, int b)
{
    if (a < b)
        return a;
    return b;
}

float max_float(float a, float b)
{
    if (a > b)
        return a;
    return b;
}
float min_float(float a, float b)
{
    if (a < b)
        return a;
    return b;
}

void toggle_route(int x1, int y1, int x, int y, int dim_c, int dim_r)
{
    if(x < 0 || x >= dim_c || y < 0 || y >= dim_r) {
        printf("KO\n");
        return;
    }
    if(x1 < 0 || x1 >= dim_c || y1 < 0 || y1 >= dim_r) {
        printf("KO\n");
        return;
    }

    hexagon * h = &(map[y1][x1]);

    int found = 0;
    route_node * list = h->routes;
    route_node * prev = NULL;


    int sum_routes_cost = h->cost;
    int dim = 1;

    while(list) {
        if (list->coord.x == x && list->coord.y == y)
        {
            if(prev) {
                prev->next = list->next;
            } else {
                h->routes = list->next;
            }

            free(list);
            found = 1;
            break;
        }

        sum_routes_cost += list->cost;
        dim++;

        prev = list;
        list = list->next;
    }
    

    if (found == 0 && dim<MAX_AIR_ROUTE + 1) {
        route_node * new_route = malloc(sizeof(route_node));
        new_route->coord.x = x;
        new_route->coord.y = y;
        new_route->cost = sum_routes_cost / dim;

        new_route->next = h->routes;
        h->routes = new_route;
        fprintf(output,"OK\n");
        libera_cache();

    } else if (found == 1) {
        libera_cache();
        fprintf(output,"OK\n");
    }
    else {
        fprintf(output,"KO\n");
    }
}

void insert_heap(min_heap * heap, int costo, int x, int y) {
    if(heap->dim_q == heap->head_q) {
        heap->dim_q *= 1.5;
        heap->q = realloc(heap->q, sizeof(node) * heap->dim_q);
    }

    heap->q[heap->head_q].coord.x = x;
    heap->q[heap->head_q].coord.y = y;
    heap->q[heap->head_q].cost_from_start = costo;
    heap->head_q++;

    int curr = heap->head_q -1;
    while(curr != 0) {
        if(heap->q[curr].cost_from_start < heap->q[parent(curr)].cost_from_start) {
            node tmp = heap->q[curr];
            heap->q[curr] = heap->q[parent(curr)];
            heap->q[parent(curr)] = tmp;
            
            curr = parent(curr);
        } else {
            break;
        }
    }

}


int apply_cost_bound(int c)
{
    return min_int(100, max_int(0, c));
}

void min_heapify(node* nodes, int dim, int index){
    int l=left(index);
    int r=right(index);
    int min;
    node tmp;

    if(l< dim && nodes[l].cost_from_start<nodes[index].cost_from_start){min = l;}
    else min = index;
    if(r< dim && nodes[r].cost_from_start<nodes[min].cost_from_start) min = r;
    if(min!=index){
        tmp = nodes[min];
        nodes[min] = nodes[index];
        nodes[index] = tmp;
        min_heapify(nodes, dim, min);
    }  
}

int left(int i){return i*2 + 1;}
int right(int i){return i*2+ 2;}
int parent(int i){return i/2;}


node heap_extract_min(node *nodes, int* dim){
    if(*dim<1) fprintf(output,"Error:underflow");
    node min = nodes[0];
    nodes[0] = nodes[(*dim) - 1];
    (*dim)--;
    min_heapify(nodes, *dim, 0);
    return min;
}

void neighboord_calculate(int x, int y) {

    if(y % 2 == 0) {
        frontier_g[0].x = x + 1; frontier_g[0].y = y;
        frontier_g[1].x = x; frontier_g[1].y = y - 1;
        frontier_g[2].x = x - 1; frontier_g[2].y = y - 1;
        frontier_g[3].x = x - 1; frontier_g[3].y = y;
        frontier_g[4].x = x - 1; frontier_g[4].y = y + 1;
        frontier_g[5].x = x; frontier_g[5].y = y + 1;
    } else {
        frontier_g[0].x = x + 1; frontier_g[0].y = y;
        frontier_g[1].x = x + 1; frontier_g[1].y = y - 1;
        frontier_g[2].x = x; frontier_g[2].y = y - 1;
        frontier_g[3].x = x - 1; frontier_g[3].y = y;
        frontier_g[4].x = x; frontier_g[4].y = y + 1;
        frontier_g[5].x = x + 1; frontier_g[5].y = y + 1;
    }

}