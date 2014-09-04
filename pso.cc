#include "pso.h"


void normalize(std::vector<double> &p);
int transition(Cell* mycell);


void Particle::updateCurrent(PlanMap* gbest)
{
    double m = this->swarm->momentum;
    double c1 = this->swarm->c1;
    double c2 = this->swarm->c2;
    Random* r1 = this->swarm->r1;
    Random* r2 = this->swarm->r2;
    int xsize = this->current->xsize;
    int ysize = this->current->ysize;
    Map<int> temp(xsize, ysize, 0);

    for (int i = 0; i < current->size(); ++i) {
        Cell* mycell = current->at(i);
        Cell* pbest_cell = pbest->at(i);
        Cell* gbest_cell = gbest->at(i);

        int myvalue = mycell->value;
        int pbest_value = pbest_cell->value;
        int gbest_value = gbest_cell->value;
		
	if (mycell->type == kBackgroundCell) continue;
        else if (mycell->type == kExcludedCell) continue;
        else if (mycell->type == kDeterminedCell) {
            continue;
        }

        // update velocity
        // v(i+1) = m*v(i) + c1*r1*(pbest-x(i)) + c2*r2*(gbest-x(i))
        for (int j = 0; j < mycell->transP.size(); ++j) {            
		mycell->transP.at(j) *= m;
        }		
	mycell->transP.at(pbest_value - 1) += c1 * r1->nextDouble();
	mycell->transP.at(gbest_value - 1) += c2 * r2->nextDouble();
        normalize(mycell->transP);

        // update position
        temp.at(i) = transition(mycell);
    }
    
    this->current->assignValue(temp);
    this->updatePbest();
}