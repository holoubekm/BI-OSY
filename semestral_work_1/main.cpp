#ifndef __PROGTEST__
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <iomanip>
#include <vector>
#include <queue>
#include <stack>
#include <deque>
#include <pthread.h>
#include <semaphore.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#if defined (__cplusplus) && __cplusplus > 199711L
/* C++ 11 */
#include <thread>
#include <mutex> 
#include <condition_variable>
#endif /* __cplusplus */
using namespace std;

struct TRect
{
	int m_X;
	int m_Y;
	int m_W;
	int m_H;
};

struct TCostProblem
{
	int ** m_Values;
	int m_Size;
	int m_MaxCost;
	void (* m_Done) (const TCostProblem *, const TRect *);
	int m_Area;
};

struct TCrimeProblem
{
	double ** m_Values;
	int m_Size;
	double m_MaxCrime;
	void (* m_Done) (const TCrimeProblem *, const TRect *);
	int m_Area;
};

#endif /* __PROGTEST__ */

class Job;
class CrimeProblem;
class CostProblem;

void add_job(Job* job);

int numThreads = 0;
pthread_mutex_t m_buf;
pthread_mutex_t m_kill;
int tasksCnt;
sem_t s_free;
sem_t s_full;
stack<Job*> buf;
int finished = 0;

struct FPtr
{
	const TCrimeProblem*(*getCrime)(void);
	const TCostProblem*(*getCost)(void);
};


bool FindByCost (int** values, int size, int maxCost, TRect* res)
{
	long** SAT = new long*[size + 1];
	for(int x = 0; x < size + 1; x++)
		SAT[x] = new long[size + 1];

	bool found = false;

	for(int y = 0; y < size + 1; y++)
	{
		SAT[y][0] = 0;
		SAT[0][y] = 0;
	}

	long row = 0, col = 0;
	for(int x = 1; x < size + 1; x++)
	{
		row += values[x - 1][0];
		SAT[x][1] = row;
		col += values[0][x - 1];
		SAT[1][x] = col;
	}

	for(int y = 2; y < size + 1; y++)
	{
		for(int x = 2; x < size + 1; x++)
		{
			SAT[y][x] = values[y - 1][x - 1] + SAT[y - 1][x] + SAT[y][x - 1] - SAT[y - 1][x - 1];
		}
	}

	long area = -1;
	// long maxCost = 6;
	for(int y = 1; y < size + 1; y++)
	{
		for(int x = 1; x < size + 1; x++)
		{
			for(int h = 0; h < size - y + 1; h++)
			{
				for(int w = 0; w < size - x + 1; w++)
				{
					long a = SAT[y - 1][x - 1];
					long b = SAT[y - 1][x + w];
					long c = SAT[y + h][x - 1];
					long d = SAT[y + h][x + w];
					long sum = d - b - c + a;

					long curArea = (w + 1) * (h + 1);
					if(sum <= maxCost && curArea > area)
					{
						area = curArea;
						res->m_X = x - 1;
						res->m_Y = y - 1;
						res->m_W = w + 1;
						res->m_H = h + 1;
						found = true;
					}
				}
			}
		}
	}

	for(int x = 0; x < size + 1; x++)
		delete[] SAT[x];
	delete[] SAT;
	return found;
}

bool FindByCrime (double ** values, int size, double maxCrime, TRect * res)
{
	long** SAT = new long*[size + 1];
	for(int x = 0; x < size + 1; x++)
		SAT[x] = new long[size + 1];


	for(int y = 0; y < size + 1; y++)
	{
		SAT[y][size] = 0;
		SAT[0][y] = 0;
	}

	for(int x = 0; x < size; x++)
	{
		SAT[1][x] = values[0][x] <= maxCrime ? 1 : 0;
	}

	for(int y = 2; y < size + 1; y++)
	{
		for(int x = 0; x < size; x++)
		{
			SAT[y][x] = SAT[y - 1][x] + (values[y - 1][x] <= maxCrime ? 1 : 0);
		}
	}

	long max = 0;
	long temp = 0;
	long start = 0;
	bool found = false;
	for(int y1 = 1; y1 < size + 1; y1++)
	{
		for(int y2 = y1; y2 < size + 1; y2++)
		{
			temp = 0;
			start = -1;
			for(int x = 0; x < size + 1; x++)
			{
				for(int x = 0; x < 10; x++);
				if(SAT[y2][x] - SAT[y1 - 1][x] == y2 - y1 + 1)
				{
					temp += y2 - y1 + 1;
					if(start == -1)
					{
						start = x;
					}
				}
				else 
				{
					if(temp > max)
					{
						max = temp;

						res->m_X = start;
						res->m_Y = (y1 - 1);
						res->m_W = (x - start);
						res->m_H = (y2 - y1 + 1);
						found = true;
					}
				temp = 0;
				start = -1;
				}
			}
		}
	}

	for(int x = 0; x < size + 1; x++)
		delete[] SAT[x];
	delete[] SAT;
	return found;
}


static const int delta_Row = 30;

class Problem
{
public:
	Problem()
	{
		SAT = NULL;
		solvedCnt = 0;
		solved = false;
		size = 0;
		maxRow = 0;
		maxArea = 0;
		rect = new TRect();
	}

	virtual ~Problem() 
	{
		delete rect;
	};

	virtual bool solve(int pos) { return true; }
	virtual void prepare() {};
	virtual void done() {};


	mutex localLock;
	// mutex m_helpers;
	// condition_variable cv_helpers;

	TRect* rect;
	long** SAT;

	int solvedCnt;
	int maxCnt;

	// int curRow;
	int maxRow;
	int maxArea;
	bool solved;
	int size;
	// int helpers;
	// bool isLast;
};

class CrimeProblem : public Problem
{
public:
	CrimeProblem(const TCrimeProblem* pprob) : prob(pprob) { }

	void prepare()
	{
		// curRow = 1;
		size = prob->m_Size;
		maxCnt = (size + (delta_Row - 1)) / delta_Row;
		//cout << "maxCnt: " << maxCnt << endl;
		maxRow = size + 1;

		SAT = new long*[size + 1];
		for(int x = 0; x < size + 1; x++)
			SAT[x] = new long[size + 1];


		for(int y = 0; y < size + 1; y++)
		{
			SAT[y][size] = 0;
			SAT[0][y] = 0;
		}

		for(int x = 0; x < size; x++)
		{
			SAT[1][x] = prob->m_Values[0][x] <= prob->m_MaxCrime ? 1 : 0;
		}

		for(int y = 2; y < size + 1; y++)
		{
			for(int x = 0; x < size; x++)
			{
				SAT[y][x] = SAT[y - 1][x] + (prob->m_Values[y - 1][x] <= prob->m_MaxCrime ? 1 : 0);
			}
		}
	}

	bool solve(int pos)
	{
		int end = pos + delta_Row;
		if(end >= maxRow)
			end = maxRow;

		TRect rec;
		long start = 0;
		long area = -1;
		long temp = 0;
		for(int y1 = pos; y1 < end; y1++)
		{	
			for(int y2 = y1; y2 < size + 1; y2++)
			{
				temp = 0;
				start = -1;
				for(int x = 0; x < size + 1; x++)
				{
					if(numThreads == 1)
						for(int x = 0; x < 30; x++);

					if(SAT[y2][x] - SAT[y1 - 1][x] == y2 - y1 + 1)
					{
						temp += y2 - y1 + 1;
						if(start == -1)
						{
							start = x;
						}
					}
					else 
					{
						if(temp > area)
						{
							area = temp;
							rec.m_X = start;
							rec.m_Y = (y1 - 1);
							rec.m_W = (x - start);
							rec.m_H = (y2 - y1 + 1);
						}
					temp = 0;
					start = -1;
					}
				}
			}
		}

		localLock.lock();
		if(area != -1 && area > maxArea)
		{
			maxArea = area;
			rect->m_X = rec.m_X;
			rect->m_Y = rec.m_Y;
			rect->m_W = rec.m_W;
			rect->m_H = rec.m_H;
			solved = true;
		}
		bool last = ++solvedCnt >= maxCnt;
		localLock.unlock();

		return last;
	}

	void done()
	{
		prob->m_Done(prob, solved ? rect : NULL);
		for(int x = 0; x < size + 1; x++)
			delete[] SAT[x];
		delete[] SAT;
	}

	const TCrimeProblem* prob;
};

class CostProblem : public Problem
{
public:
	CostProblem(const TCostProblem* pprob) { prob = pprob; }
	
	void prepare()
	{
		size = prob->m_Size;
		// curRow = 1;
		maxCnt = (size + (delta_Row - 1)) / delta_Row;
		//cout << "maxCnt: " << maxCnt << endl;
		maxRow = size + 1;

		SAT = new long*[size + 1];
		for(int x = 0; x < size + 1; x++)
			SAT[x] = new long[size + 1];

		for(int y = 0; y < size + 1; y++)
		{
			SAT[y][0] = 0;
			SAT[0][y] = 0;
		}

		long row = 0, col = 0;
		for(int x = 1; x < size + 1; x++)
		{
			row += prob->m_Values[x - 1][0];
			SAT[x][1] = row;
			col += prob->m_Values[0][x - 1];
			SAT[1][x] = col;
		}

		for(int y = 2; y < size + 1; y++)
		{
			for(int x = 2; x < size + 1; x++)
			{
				SAT[y][x] = prob->m_Values[y - 1][x - 1] + SAT[y - 1][x] + SAT[y][x - 1] - SAT[y - 1][x - 1];
			}
		}
	}

	bool solve(int pos)
	{
		int end = pos + delta_Row;
		if(end >= maxRow)
			end = maxRow;

		//cout << "start: " << pos << ", end: " << end << endl;
		TRect rec;
		long area = -1;
		for(int y = pos; y < end; y++)
		{
			for(int x = 1; x < size + 1; x++)
			{
				for(int h = 0; h < size - y + 1; h++)
				{
					for(int w = 0; w < size - x + 1; w++)
					{
						if(numThreads == 1)
							for(int x = 0; x < 10; x++);

						long a = SAT[y - 1][x - 1];
						long b = SAT[y - 1][x + w];
						long c = SAT[y + h][x - 1];
						long d = SAT[y + h][x + w];
						long sum = d - b - c + a;

						long curArea = (w + 1) * (h + 1);
						if(sum <= prob->m_MaxCost && curArea > area)
						{
							area = curArea;
							rec.m_X = x - 1;
							rec.m_Y = y - 1;
							rec.m_W = w + 1;
							rec.m_H = h + 1;
						}
					}
				}
			}
		}
		localLock.lock();
		if(area != -1 && area > maxArea)
		{
			maxArea = area;
			rect->m_X = rec.m_X;
			rect->m_Y = rec.m_Y;
			rect->m_W = rec.m_W;
			rect->m_H = rec.m_H;
			solved = true;
		}
		bool last = ++solvedCnt >= maxCnt;
		localLock.unlock();

		return last;
	}

	void done()
	{
		// ////cout << "Solved: " << solved << endl;
		// ////cout << maxArea << endl;
		prob->m_Done(prob, solved ? rect : NULL);
		for(int x = 0; x < size + 1; x++)
			delete[] SAT[x];
		delete[] SAT;
	}

	const TCostProblem* prob;
};



class Job
{
public:
	Job() : isLast(false), isMain(false)
	{}

	Job(bool pisLast) : isLast(pisLast), isMain(false)
	{}
 
	virtual ~Job() { }
	virtual void solve() { }

	bool isLast;
	bool isMain;
};

class CrimeJob : public Job
{
public:
	CrimeJob(CrimeProblem* pprob, int pfrom) : prob(pprob), from(pfrom) {}

	void solve()
	{
		bool last = prob->solve(from);
		if(last)
		{
			prob->done();
			delete prob;

			pthread_mutex_lock(&m_kill);
			tasksCnt--;
			bool kill = tasksCnt == 0 && finished == 2;
			pthread_mutex_unlock(&m_kill);

			if(kill)
			{
				for(int x = 0; x < numThreads; x++)
				{
					//cout << "killing" << endl;
					sem_wait(&s_free);
					add_job(new Job(true));
				}
			}
		}
	}
	~CrimeJob() {}
	CrimeProblem* prob;
	int from;
};

class CostJob : public Job
{
public:
	CostJob(CostProblem* pprob, int pfrom) : prob(pprob), from(pfrom) {}
	void solve()
	{
		// //cout << "from: " << from << endl;
		bool last = prob->solve(from);
		if(last)
		{
			//cout << "Tadyyyyyyyyyyyyyy" << endl;
			prob->done();
			delete prob;

			pthread_mutex_lock(&m_kill);
			tasksCnt--;
			bool kill = tasksCnt == 0 && finished == 2;
			pthread_mutex_unlock(&m_kill);

			if(kill)
			{
				for(int x = 0; x < numThreads; x++)
				{
					//cout << "killing" << endl;
					sem_wait(&s_free);
					add_job(new Job(true));
				}
			}
		}
	}
	~CostJob() {}
	CostProblem* prob;
	int from;
};

class PrepareCrimeJob : public Job
{
public:
	PrepareCrimeJob(CrimeProblem* pprob) : prob(pprob)
	{};

	void solve()
	{
		prob->prepare();
		int x;
		int size = prob->size;
		for(x = 1; x < size + 1; x += delta_Row){
			//cout << "x: " << x << endl;
			add_job(new CrimeJob(prob, x));
		}
		if(x < size + 1)
		{
			//cout << "size: " << prob->size << endl;
			//cout << "x: " << (prob->size - x + 1) << endl;
			//cout << "y: " << (((prob->size + 1) / delta_Row) * delta_Row) << endl;
			add_job(new CrimeJob(prob, ((prob->size + 1) / delta_Row) * delta_Row));
		}
	}
	~PrepareCrimeJob() {}
	CrimeProblem* prob;
};


class PrepareCostJob : public Job
{
public:
	PrepareCostJob(CostProblem* pprob) : prob(pprob)
	{};

	void solve()
	{
		//cout << "Preparing CostJob" << endl;
		prob->prepare();
		//cout << "Prepared CostJob" << endl;
		int x;
		int size = prob->size;
		for(x = 1; x < size + 1; x += delta_Row){
			//cout << "x: " << x << endl;
			add_job(new CostJob(prob, x));
		}
		if(x < size + 1)
		{
			//cout << "size: " << prob->size << endl;
			//cout << "x: " << (prob->size - x + 1) << endl;
			//cout << "y: " << (((prob->size + 1) / delta_Row) * delta_Row) << endl;
			add_job(new CostJob(prob, ((prob->size + 1) / delta_Row) * delta_Row));
		}
	}
	~PrepareCostJob() {}
	CostProblem* prob;
};





void add_job(Job* job)
{
	pthread_mutex_lock(&m_buf);
		buf.push(job);
	pthread_mutex_unlock(&m_buf);
	sem_post(&s_full);
}

void* f_cost_producer(void* ptr)
{
	FPtr* func = (FPtr*)ptr;
	while(true)
	{
		const TCostProblem* problem = func->getCost();
		Job* wrapper = NULL;
		if(problem != NULL)
		{
			wrapper = new PrepareCostJob(new CostProblem(problem));
			wrapper->isMain = true;
		}
		else
		{
			pthread_mutex_lock(&m_kill);
			finished++;
			pthread_mutex_unlock(&m_kill);
			return NULL;
		}
		//cout << "here 2" << endl;
		
		sem_wait(&s_free);
		
		pthread_mutex_lock(&m_kill);
		tasksCnt++;
		pthread_mutex_unlock(&m_kill);

		pthread_mutex_lock(&m_buf);
			//cout << "Adding job" << endl;
			//cout << "isLast: " << wrapper->isLast << endl;
			//cout << "isMain: " << wrapper->isMain << endl;
			buf.push(wrapper);
		// tasksCnt++;
		pthread_mutex_unlock(&m_buf);
		sem_post(&s_full);
	}
	return NULL;
}


void* f_crime_producer(void* ptr)
{
	FPtr* func = (FPtr*)ptr;
	while(true)
	{
		const TCrimeProblem* problem = func->getCrime();
		Job* wrapper = NULL;
		if(problem != NULL)
		{
			wrapper = new PrepareCrimeJob(new CrimeProblem(problem));
			wrapper->isMain = true;
		}
		else
		{
			pthread_mutex_lock(&m_kill);
			finished++;
			pthread_mutex_unlock(&m_kill);
			return NULL;
		}

		sem_wait(&s_free);
		
		pthread_mutex_lock(&m_kill);
		tasksCnt++;
		pthread_mutex_unlock(&m_kill);


		pthread_mutex_lock(&m_buf);
			//cout << "Adding job" << endl;
			//cout << "isLast: " << wrapper->isLast << endl;
			//cout << "isMain: " << wrapper->isMain << endl;
			buf.push(wrapper);
		pthread_mutex_unlock(&m_buf);
		sem_post(&s_full);

	}
	return NULL;
}


void* f_worker(void* ptr)
{
	// intptr_t id = (intptr_t) ptr;
	Job* task = NULL;
	while(true)
	{
		sem_wait(&s_full);
		pthread_mutex_lock(&m_buf);
			task = buf.top(); buf.pop();
			//cout << "Gotta task" << endl;
		pthread_mutex_unlock(&m_buf);

		if(task->isLast)
			break;
		
		if(task->isMain)
			sem_post(&s_free);

		task->solve();
		delete task;
	}
	delete task;
	return NULL;
}

void MapAnalyzer (int threads, const TCostProblem * (* costFunc) (void), const TCrimeProblem* (* crimeFunc) (void))
{
	tasksCnt = 0;
	finished = 0;
	numThreads = threads;
	FPtr* funcs = new FPtr();
	
	funcs->getCost = costFunc;
	funcs->getCrime = crimeFunc;

	pthread_t cost_producer;
	pthread_t crime_producer;
	pthread_t* workers = new pthread_t[threads];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	pthread_mutex_init(&m_buf, NULL);
	pthread_mutex_init(&m_kill, NULL);
	sem_init(&s_free, 0, threads);
	sem_init(&s_full, 0, 0);

	pthread_create(&cost_producer, &attr, f_cost_producer, (void*) funcs);
	pthread_create(&crime_producer, &attr, f_crime_producer, (void*) funcs);
	for(intptr_t x = 0; x < threads; x++)
	{
		pthread_create(&workers[x], &attr, (void*(*)(void*)) f_worker, (void*)x);
	}
	pthread_attr_destroy(&attr);

	pthread_join(cost_producer, NULL);  
	pthread_join(crime_producer, NULL);
	for(int x = 0; x < threads; x++)
	{
		pthread_join(workers[x], NULL);
	}

	pthread_mutex_destroy(&m_buf); 
	pthread_mutex_destroy(&m_kill); 
	sem_destroy(&s_free);
	sem_destroy(&s_full);

	delete funcs;
	delete[] workers;
}

