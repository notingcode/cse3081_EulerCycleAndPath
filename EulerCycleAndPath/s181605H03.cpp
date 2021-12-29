#define _CRT_SECURE_NO_WARNINGS
using namespace std;
#include <time.h>
#include <stack>
#include "DBL.h"

//#define NO_PATH_OUT   // comment out this line for path output
void Error_Exit(const char *s);

typedef struct _Vertex {
	dblStack S;		// adj list contains edge index
	int degree;
} Vertex;

typedef struct _Edge {
	int v1, v2;
} Edge;

void graphGeneration(Vertex **V, Edge **E, int *VN, int *EN);
void adjListGenerate(Vertex *V, Edge *E, int VN, int EN);
void deallocGraph(Vertex *V, Edge *E, int VN);
int *Find_Euler(Vertex *V, Edge *E, int VN, int EN, int *flag, int *pathN);

DBList pool;	// DBL storage pool

int main() {
	int T, VN, EN;
	Vertex *V;
	Edge   *E;
	int *path;	// Euler cycle or path
	int pathN;  // path length
	int  flag;	// 0: cycle, 1: path, 2: none
	clock_t start_time, finish_time;

	scanf("%d", &T);	// read # of tests
	for (int t = 1; t <= T; t++) {	// for each test
		graphGeneration(&V, &E, &VN, &EN);

		start_time = clock(); // set the start time

		path = Find_Euler(V, E, VN, EN, &flag, &pathN); // find an Euler path or cycle

		finish_time = clock(); // set finish time
		
		double cmpt = (((double)(finish_time - start_time)) / CLK_TCK)*1000; // compute the time passed
		printf("Test= %d flag= %d VnumInCycle/Path)= %d ", t, flag, pathN);

		if (flag == 0)
			printf("Euler_cycle(exec_time= %.2f msec)\n",cmpt);
		else if (flag == 1)
			printf("Euler_path(exec_time= %.2f msec)\n", cmpt);
		else
			printf("not_Eulerian(exec_time= %.2f msec)\n", cmpt);

#ifndef NO_PATH_OUT
		if (flag != 2) {
			for (int i = 0; i < EN + 1; i++) {
				printf("%d\n", path[i]);
			}
		}
#endif
		if (flag != 2)
			delete[] path;
		deallocGraph(V, E, VN);
	}
	pool.freeDBL_pool();	// clear all the DBL elements

	return 0;
}

int *Find_Euler(Vertex *V, Edge *E, int VN, int EN, int *flag, int *pathN) {
	// input V, VN, E, EN
	// output through paramters
	//   *flag = 0 (Euler cycle), 1 (Euler path), 2 (not Eulerian)
	//   *pathN = size of path[] array
	// output by return
	//   *path = list of vertex ids found(Euler cycle or path)

	stack<int> ST;		// use STL stack as explained in the class
	int *path = NULL;
	int start_v, oddD;
	start_v = oddD = *flag = *pathN = 0;

	// Find number of odd degree vertices and non-Eulerian condition
	for (int v = 0; v < VN && *flag != 2; ++v) {
		if (V[v].degree % 2 == 1) {
			oddD++;
			start_v = v;
		}
		if (oddD > 2 || V[v].degree == 0)
			*flag = 2;
	}
	if (oddD == 1)
		*flag = 2;
	if (oddD == 2)
		*flag = 1;

	// The graph is connected and has Euler cycle or path
	if (*flag != 2) {
		DBL* e;
		int adj_v, curr_v;
		path = new int[EN + 1];
		
		ST.push(start_v);
		while (ST.empty() == false) {
			curr_v = ST.top();
			// If there is no unvisited incident edges, add to path
			if (V[curr_v].degree == 0) {
				path[(*pathN)++] = curr_v;
				ST.pop();
			}
			else {
				e = V[curr_v].S.pop();
				V[curr_v].degree--;
				// Find adjacent vertex of the current vertex
				(E[e->d].v1 == curr_v) ? adj_v = E[e->d].v2 : adj_v = E[e->d].v1;
				// Remove the identical incident edge element from adjacent vertex
				V[adj_v].S.remove(e->twin);
				V[adj_v].degree--;
				ST.push(adj_v);
				// Move poped/removed edge to pool
				pool.freeDBL(e);
				pool.freeDBL(e->twin);
			}
		}
	}

	return path;
}

void deallocGraph(Vertex *V, Edge *E, int VN) {
	DBL *p;

	// Iterate through all vertices and pop all stack elements to pool
	for (int k = 0; k < VN; ++k) {
		while (V[k].S.empty() == false) {
			p = V[k].S.pop();
			pool.freeDBL(p);
		}
	}

	delete[] V;
	delete[] E;
}

void graphGeneration(Vertex **V, Edge **E, int *VN, int *EN) {
	scanf("%d %d", VN, EN);	// read # of vertices and edges
	//allocVandEarray(V, E, *VN, *EN);	// vertex and edge array allocation

	*V = new Vertex[*VN];
	*E = new Edge[*EN];
	if (*V == NULL || *E == NULL) {
		Error_Exit("Memory Allocation Error!");
	}

	for (int v = 0; v < *VN; v++) {
		(*V)[v].degree = 0;
	}
	for (int e = 0; e < *EN; e++) {
		scanf("%d %d", &((*E)[e].v1), &((*E)[e].v2));	// read edge information
		++((*V)[(*E)[e].v1].degree);
		++((*V)[(*E)[e].v2].degree);
	}
	adjListGenerate(*V, *E, *VN, *EN);	// create adj lust in G=(V,E)
}

void adjListGenerate(Vertex *V, Edge *E, int VN, int EN) {
	// Construct adjacent list in V array
	int v1, v2;
	DBL *adjE1, *adjE2;

	for (int k = 0; k < EN; ++k) {
		v1 = E[k].v1;
		v2 = E[k].v2;
		adjE1 = pool.allocDBL();
		adjE2 = pool.allocDBL();
		
		adjE1->twin = adjE2;
		adjE1->d = k;
		adjE2->twin = adjE1;
		adjE2->d = k;

		V[v1].S.push(adjE1);
		V[v2].S.push(adjE2);
	}
}

void Error_Exit(const char *s) {
	printf("%s", s);
	exit(-1);
}

DBL *DBList::allocDBL(void) {		// allocate a DBL element
	DBL *p;

	if (DBL_pool == NULL) {
		p = new DBL;
		if (p == NULL) {
			Error_Exit("DBL memory alloc error(allocDBL)");
		}
		UsedMemoryForDBLs += sizeof(DBL);
	}
	else {
		p = DBL_pool;
		DBL_pool = DBL_pool->rp;
	}
	p->d = NONE;
	p->rp = p->lp = p->twin = NULL;
	++DBL_cnt;

	return(p);
}

void DBList::freeDBL(DBL *p) {
	// move p to pool

	if (p->d == NONE) {
		Error_Exit("This element is already freed(Free_DBL).");
	}
	p->rp = DBL_pool;
	DBL_pool = p;
	--DBL_cnt;		// reduce # of active DBL elements by one 
}

void DBList::freeDBL_pool(void) {
	DBL *p = DBL_pool;

	while (p != NULL) {
		DBL_pool = p->rp;	// get next pointer
		delete p;
		p = DBL_pool;
		UsedMemoryForDBLs -= sizeof(DBL);
	}
	if (DBL_cnt != 0) {
		Error_Exit("Non-zero DBL_cnt after cleanup.");
	}
}

void dblStack::push(DBL *p) {
	if (ST != NULL) {
		ST->lp = p;
	}
	p->rp = ST;
	p->lp = NULL;
	ST = p;
}

DBL *dblStack::pop(void) {	// remove and return p in front of Q
	DBL *p;
	p = ST;
	if (ST->rp == NULL)
		ST = NULL;
	else {
		ST = ST->rp;
		ST->lp = NULL;
	}

	return p;
}

void dblStack::remove(DBL *p) {	// delete p from ST
	if (ST == p) {
		ST = ST->rp;
		if (ST != NULL) {
			ST->lp = NULL;
		}
	}
	else {
		(p->lp)->rp = p->rp;
		if (p->rp != NULL) {
			(p->rp)->lp = p->lp;
		}
	}

}

DBL *dblStack::top(void) {
	return ST;
}

bool dblStack::empty(void) {
	if (ST == NULL)
		return true;
	else
		return false;
}
