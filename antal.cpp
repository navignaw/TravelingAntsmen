// Sequential ant algorithm for Traveling Salesman Problem

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include "CycleTimer.h"
#include "ants.h"

// stores ant data (visited cities and current path)
struct antType {
  int curCity, nextCity, pathIndex;
  int tabu[MAX_CITIES];
  int path[MAX_CITIES];
  float tourLength;
};

// runtime structures and global variables
antType ants[MAX_ANTS];

EdgeMatrix *dist;
EdgeMatrix phero;

float best = (float) MAX_TOUR;
int bestIndex;

// initializes the entire graph
void init() {
  int from, to, ant;

  // creating cities
  for (from = 0; from < MAX_CITIES; from++) {
    for (to = 0; to < from; to++) {
      phero.set_edge(from, to, INIT_PHER);
    }
  }

  // initializing the ants
  to = 0;
  for (ant = 0; ant < MAX_ANTS; ant++) {
    if (to == MAX_CITIES) {
      to = 0;
    }

    ants[ant].curCity = to++;
    for (from = 0; from < MAX_CITIES; from++) {
      ants[ant].tabu[from] = 0;
      ants[ant].path[from] = -1;
    }

    ants[ant].pathIndex = 1;
    ants[ant].path[0] = ants[ant].curCity;
    ants[ant].nextCity = -1;
    ants[ant].tourLength = 0;

    // load first city into tabu list
    ants[ant].tabu[ants[ant].curCity] = 1;
  }
}

// reinitialize all ants and redistribute them
void restartAnts() {
  int ant, i, to = 0;

  for (ant = 0; ant < MAX_ANTS; ant++) {
    ants[ant].nextCity = -1;
    ants[ant].tourLength = 0.0;

    for (i = 0; i < MAX_CITIES; i++) {
      ants[ant].tabu[i] = 0;
      ants[ant].path[i] = -1;
    }

    ants[ant].curCity = to++;
    ants[ant].pathIndex = 1;
    ants[ant].path[0] = ants[ant].curCity;
    ants[ant].tabu[ants[ant].curCity] = 1;
  }
}

float antProduct(int from, int to) {
  return (pow(phero.edge(from, to), ALPHA) * pow(1.0 / dist->edge(from, to), BETA));
}

int selectNextCity(int ant) {
  int from = ants[ant].curCity;
  float sum = 0.0;

  for (int to = 0; to < MAX_CITIES; to++) {
    if (ants[ant].tabu[to] == 0) {
      sum += antProduct(from, to);
    }
  }

  if (sum == 0) {
    printf("ERROR: zero sum in selectNextCity\n");
    return 0;
  }

  float acc = 0;
  float luckyNumber = (float)rand() / RAND_MAX;

  for (int to = 0; to < MAX_CITIES; to++) {
    if (ants[ant].tabu[to] == 0) {
      acc += antProduct(from, to) / sum;

      if (acc >= luckyNumber) {
        return to;
      }
    }
  }

  //should not get here
  printf("acc: %f, luckyNumber: %f\n", acc, luckyNumber);
  printf("ERROR: failed to select next city\n");
  return 0;
}

int simulateAnts() {
  int k;
  int moving = 0;

  for (k = 0; k < MAX_ANTS; k++) {
    // check if there are any more cities to visit

    if(ants[k].pathIndex < MAX_CITIES) {
      ants[k].nextCity = selectNextCity(k);
      ants[k].tabu[ants[k].nextCity] = 1;
      ants[k].path[ants[k].pathIndex++] = ants[k].nextCity;

      ants[k].tourLength += dist->edge(ants[k].curCity, ants[k].nextCity);

      //handle last case->last city to first
      if (ants[k].pathIndex == MAX_CITIES) {
        ants[k].tourLength += dist->edge(ants[k].path[MAX_CITIES -1], ants[k].path[0]);
      }

      ants[k].curCity = ants[k].nextCity;
      moving++;
    }
  }

  return moving;
}

// Updating trails
void updateTrails()
{
  int from, to, i, ant;

  // Pheromone Evaporation
  for (from = 0; from < MAX_CITIES; from++) {
    for (to = 0; to < from; to++) {
      phero.set_edge(from, to, phero.edge(from, to) * (1.0 - RHO));

      if (phero.edge(from, to) < 0.0) {
        phero.set_edge(from, to, INIT_PHER);
      }
    }
  }

  //Add new pheromone to the trails
  for (ant = 0; ant < MAX_ANTS; ant++) {
    for (i = 0; i < MAX_CITIES; i++) {
      if (i < MAX_CITIES - 1) {
        from = ants[ant].path[i];
        to = ants[ant].path[i+1];
      } else {
        from = ants[ant].path[i];
        to = ants[ant].path[0];
      }

      phero.set_edge(from, to, phero.edge(from, to) + (QVAL / ants[ant].tourLength));
    }
  }

  for (from = 0; from < MAX_CITIES; from++) {
    for (to = 0; to < from; to++) {
      phero.set_edge(from, to, phero.edge(from, to) * RHO);
    }
  }
}

float seq_ACO(EdgeMatrix *d, int *bestPath) {
  dist = d;
  int curTime = 0;
  int success = 0;
  //cout << "S-ACO:";
  //cout << "MaxTime=" << MAX_TIME;

  srand(time(NULL));

  init();

  while (curTime++ < MAX_TIME) {
    bestIndex = -1;
    success = simulateAnts();
    if (success == 0) {
      for (int ant = 0; ant < MAX_ANTS; ant++) {
        if (ants[ant].tourLength < best) {
          best = ants[ant].tourLength;
          printf("new best: %f\n", best);
          bestIndex = ant;
        }
      }

      if (bestIndex != -1) {
        memcpy(bestPath, ants[bestIndex].path, sizeof(int) * MAX_CITIES);
      }

      updateTrails();

      if (curTime != MAX_TIME) {
        restartAnts();
      }

      //cout << "\nTime is " << curTime << "(" << best << ")";
    }
  }

  return best;
}
