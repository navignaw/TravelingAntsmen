// Sequential ant algorithm for Traveling Salesman Problem

#include <iostream>
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>
#include <fstream>
//#include <cv.h>
//#include <highgui.h>

#include "CycleTimer.h"
#include "ants.h"

// runtime structures and global variables
// TODO: store these in a global struct or something?
cityType *cities;
antType ants[MAX_ANTS];

double dist[MAX_CITIES][MAX_CITIES];
double phero[MAX_CITIES][MAX_CITIES];

double best = (double) MAX_TOUR;
int bestIndex;

// initializes the entire graph
void init() {
  int from, to, ant;

  // creating cities
  for (from = 0; from < MAX_CITIES; from++) {
    for (to = 0; to < MAX_CITIES; to++) {
      dist[from][to] = 0.0;
      phero[from][to] = INIT_PHER;
    }
  }

  // computing distance
  for (from = 0; from < MAX_CITIES; from++) {
    for (to = 0; to < MAX_CITIES; to++) {
      if (to != from && dist[from][to] == 0.0) {
        int xd = pow(abs(cities[from].x - cities[to].x), 2);
        int yd = pow(abs(cities[from].y - cities[to].y), 2);

        dist[from][to] = sqrt(xd + yd);
        dist[to][from] = dist[from][to];
      }
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
    if (ants[ant].tourLength < best) {
      best = ants[ant].tourLength;
      bestIndex = ant;
    }

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

double antProduct(int from, int to) {
  return (pow(phero[from][to], ALPHA) * pow((1.0 / dist[from][to]), BETA));
}

int selectNextCity(int ant) {
  int from, to;
  double denom = 0.0;

  from = ants[ant].curCity;

  for (to = 0; to < MAX_CITIES; to++) {
    if (ants[ant].tabu[to] == 0) {
      denom += antProduct(from, to);
    }
  }

  for (to = 0; to < MAX_CITIES; to++) {
    double p;

    if (ants[ant].tabu[to] == 0) {
      p = antProduct(from, to) / denom;

      double x = ((double)rand() / RAND_MAX);
      if (x < p) {
        return to;
      }
    }
  }

  // TODO: This might screw up correctness.
  // If we have yet to pick an edge, select one at random
  return rand() % MAX_CITIES;
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

      ants[k].tourLength += dist[ants[k].curCity][ants[k].nextCity];

      //handle last case->last city to first
      if (ants[k].pathIndex == MAX_CITIES) {
        ants[k].tourLength += dist[ants[k].path[MAX_CITIES -1]][ants[k].path[0]];
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
    for (to = 0; to < MAX_CITIES; to++) {
      if (from != to) {
        phero[from][to] *= 1.0 - RHO;

        if (phero[from][to] < 0.0) {
          phero[from][to] = INIT_PHER;
        }
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

      phero[from][to] += (QVAL / ants[ant].tourLength);
      phero[to][from] = phero[from][to];
    }
  }

  for (from = 0; from < MAX_CITIES; from++) {
    for (to = 0; to < MAX_CITIES; to++) {
      phero[from][to] *= RHO;
    }
  }
}

void emitDataFile(int bestIndex)
{
  std::ofstream f1;
  f1.open("Data.txt");
  antType antBest;
  antBest = ants[bestIndex];
  //f1<<antBest.curCity<<" "<<antBest.tourLength<<"\n";
  for (int i = 0; i < MAX_CITIES; i++) {
    f1 << antBest.path[i] << " ";
  }

  f1.close();

  f1.open("city_data.txt");
  for (int i = 0; i < MAX_CITIES; i++) {
    f1 << cities[i].x << " " << cities[i].y << "\n";
  }
  f1.close();
}


void seq_ACO(cityType *c) {
  cities = c;
  int curTime = 0;

  //cout << "S-ACO:";
  //cout << "MaxTime=" << MAX_TIME;

  srand(time(NULL));

  init();

  while (curTime++ < MAX_TIME) {
    if (simulateAnts() == 0) {
      updateTrails();

      if (curTime != MAX_TIME) {
        restartAnts();
      }

      //cout << "\nTime is " << curTime << "(" << best << ")";
    }
  }

  //cout << "\nSACO: Best tour = " << best << endl;
  emitDataFile(bestIndex);
}

