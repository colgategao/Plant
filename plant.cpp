// plant.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include<iostream>
#include<algorithm>
#include<random>
#include<vector>
#include<queue>
#include<map>

using namespace std;
const int REGULAR = 0;
const int URGENT = 1;
const int EXP = 2;

class Order
{
public:
	int StartDay = 0;
	int EndDay = 0;
	int Type = REGULAR;
	bool Exp = false;
public:
	Order(int type);
};

default_random_engine generator;
poisson_distribution<int> RegularDistribution(6); //regular arrival rate
poisson_distribution<int> UrgentDistribution(2); //urgent arrival rate

vector<Order*> AllOrderList;

int ServingDays = 1;
int ExpDays = 1;
//int MaxWaitingDays = 3;

int ServingCapacity;
queue<Order*> ServingQueue;

int BacklogCapacity;
queue<Order*> BacklogQueue;

queue<Order*> ExpQueue;

int Experiment;
int rawexperiment;
float wmup; //needs to be fload and more condition
int CurrentDay = 1;

int CurrentUrgent = 0;
int UrgentEnterServingNo = 0;
int UrgentEnterExpNo = 0;

int CurrentRegular = 0;
int BacklogEnterServingNo = 0;
int RegularEnterServingNo = 0;
int BacklogToExpNo = 0;
int RegularEnterBacklogNo = 0;
int RegularEnterExpNo = 0;
int TotalRegualrDemand = 0;
int TotalUrgentDemand = 0;

void ReadyToDeliver();
void RandomlyGenerateDemand();
void ProcessUrgent();
void ProcessRegular();
bool IsOver();
void Stat();

Order::Order(int type)
{
	this->StartDay = CurrentDay;
	this->Type = type;
	AllOrderList.push_back(this);
}

int _tmain(int argc, _TCHAR* argv[])
{
	/*cout << "==========PARAMETERS:==========" << endl;
	cout << "Serivce Capacity: " << ServingCapacity << endl;;
	cout << "Backlog Capacity: " << BacklogCapacity << endl;
	cout << "Experiment Replicate: " << Experiment<<endl;
	cout << "==========PARAMETERS END:==========" << endl;
	cout << endl;*/

	cout << "please input Service Capacity: " << endl;
	cin >> ServingCapacity;
	cout << "please input Backlog Capacity: " << endl;
	cin >> BacklogCapacity;
	cout << "please input Experiment Replicate: " << endl;
	cin >> rawexperiment;
	cout << "please input warmup period (%):" << endl;
	cin >> wmup;
	cout << endl;

	int l;
	if (wmup<1)
	{
		l = wmup*rawexperiment;
		Experiment = rawexperiment - l;
	}
	l = wmup / 100 * rawexperiment;
	Experiment = rawexperiment - l;

	
	while (true)
	{
		int a = ServingQueue.size();
		int b = BacklogQueue.size();
		int c = ExpQueue.size();
		ReadyToDeliver();
		a = ServingQueue.size();
		b = BacklogQueue.size();
		c = ExpQueue.size();
		if (CurrentDay <= Experiment)
		{
			RandomlyGenerateDemand();
		}
		else
		{
			CurrentUrgent = 0;
			CurrentRegular = 0;
		}
		ProcessUrgent();
		a = ServingQueue.size();
		b = BacklogQueue.size();
		c = ExpQueue.size();
		ProcessRegular();
		a = ServingQueue.size();
		b = BacklogQueue.size();
		c = ExpQueue.size();

		bool over = IsOver();
		if (over)
		{
			break;
		}
		
		CurrentDay++;
	}

	Stat();

	char x;
	cin >> x;
	return 0;
}

void MoveOutServing()
{
	while (ServingQueue.size() > 0)
	{
		Order* order = ServingQueue.front();
		if (CurrentDay - order->StartDay >= ServingDays)
		{
			order->EndDay = CurrentDay;
			ServingQueue.pop();
		}
		else
		{
			break;
		}
	}
}
void MoveOutExp()
{
	while (ExpQueue.size() > 0)
	{
		Order* order = ExpQueue.front();
		if (CurrentDay - order->StartDay >= ExpDays)
		{
			order->EndDay = CurrentDay;
			ExpQueue.pop();
		}
		else
		{
			break;
		}
	}
}
void ReadyToDeliver()
{
	MoveOutServing();
	MoveOutExp();
}

void RandomlyGenerateDemand()
{
	CurrentRegular = RegularDistribution(generator);
	CurrentUrgent = UrgentDistribution(generator);
}

void UrgentToServing();
void UrgentToExp();
void ProcessUrgent()
{
	UrgentToServing();
	UrgentToExp();
}

void UrgentToServing()
{
	int urgent = CurrentUrgent;
	TotalUrgentDemand += CurrentUrgent;
	int ServingV = ServingCapacity - ServingQueue.size();
	UrgentEnterServingNo = min(ServingV, urgent);

	for (int i = 0; i < UrgentEnterServingNo; i++)
	{
		Order* order = new Order(URGENT);
		ServingQueue.push(order);
	}
}
void UrgentToExp()
{
	int urgent = CurrentUrgent - UrgentEnterServingNo;
	UrgentEnterExpNo = urgent;
	for (int i = 0; i < UrgentEnterExpNo; i++)
	{
		Order* order = new Order(URGENT);
		order->Exp = true;
		ExpQueue.push(order);
	}
}

void BacklogToServing();
void RegularToServing();
void BacklogToExp();
void RegularToBacklog();
void RegularToExp();
void ProcessRegular()
{
	BacklogToServing();
	RegularToServing();
	BacklogToExp();
	RegularToBacklog();
	RegularToExp();
}
void BacklogToServing()
{
	int vanS = ServingCapacity - ServingQueue.size();
	int noB = BacklogQueue.size();
	BacklogEnterServingNo = min(vanS, noB);
	for (int i = 0; i < BacklogEnterServingNo; i++)
	{
		Order* order = BacklogQueue.front();
		BacklogQueue.pop();
		ServingQueue.push(order);
	}
}
void RegularToServing()
{
	int regular = CurrentRegular;
	TotalRegualrDemand += CurrentRegular;
	int vanS = ServingCapacity - ServingQueue.size();
	RegularEnterServingNo = min(vanS, regular);

	for (int i = 0; i < RegularEnterServingNo; i++)
	{
		Order* order = new Order(REGULAR);
		ServingQueue.push(order);
	}
}
void BacklogToExp()
{
	int regular = CurrentRegular - RegularEnterServingNo;
	int vanB = BacklogCapacity - BacklogQueue.size();
	int extra = regular - vanB;

	int count = 0;
	while (BacklogQueue.size() > 0)
	{
		if (count >= extra)
		{
			break;
		}
		Order* order = BacklogQueue.front();
		/*if (CurrentDay - order->StartDay >= MaxWaitingDays)
		{*/
			BacklogQueue.pop();
			order->Exp = true;
			ExpQueue.push(order);
			count++;
		/*}*/
		/*else
		{
			break;
		}*/
	}
	BacklogToExpNo = count;
}
//void BacklogToExp()
//{
//	int regular = CurrentRegular - RegularEnterServingNo;
//	int vanB = BacklogCapacity - BacklogQueue.size();
//	int extra = regular - vanB;
//
//	int count = 0;
//	while (true)
//	{
//		if (count >= extra)
//		{
//			break;
//		}
//
//		Order* order = BacklogQueue.front();
//		if (CurrentDay - order->StartDay > MaxWaitingDays)
//		{
//			BacklogQueue.pop();
//			order->Exp = true;
//			ExpQueue.push(order);
//			count++;
//		}
//		else
//		{
//			break;
//		}
//	}
//	BacklogToExpNo = count;
//}
void RegularToBacklog()
{
	int regular = CurrentRegular - RegularEnterServingNo;
	int vanB = BacklogCapacity - BacklogQueue.size();
	RegularEnterBacklogNo = min(regular, vanB);
	for (int i = 0; i < RegularEnterBacklogNo; i++)
	{
		Order* order = new Order(REGULAR);
		BacklogQueue.push(order);
	}
}
void RegularToExp()
{
	int regular = CurrentRegular - RegularEnterServingNo - RegularEnterBacklogNo;
	RegularEnterExpNo = regular;
	for (int i = 0; i < RegularEnterExpNo; i++)
	{
		Order* order = new Order(REGULAR);
		order->Exp = true;
		ExpQueue.push(order);
	}
}

bool IsOver()
{
	return ServingQueue.size() == 0 && BacklogQueue.size() == 0 && ExpQueue.size() == 0;
}

void StatRegularLT();
//void StatDetail();
void Stat()
{
	StatRegularLT();
	/*StatDetail();*/
}

void StatRegularLT()
{
	
	cout << "========== Regular Lead Time Statistics ==========" << endl;
	map<int, int> LT;
	//map<int, int> LT1;//new
	int total = 0;
	float ELT = 0;
	float ELT1 = 0;
	for (vector<Order*>::iterator i = AllOrderList.begin(); i != AllOrderList.end(); i++)
	{
		Order* order = *i;
		if (order->Type != REGULAR)
		{
			continue;
		}

		int lt = order->EndDay - order->StartDay;
		if (LT.find(lt) == LT.end())
		{
			LT[lt] = 0;
		}
		LT[lt]++;
		total++;
	}

	for (map<int, int>::iterator i = LT.begin(); i != LT.end(); i++)
	{
		float m = i->second /(float) TotalRegualrDemand;
		cout << "LT " << i->first << ": " << m <<"   ("<<i->second<<")"<< endl;
		ELT += i->first*m;
	}
	cout << "Expected Leadtime of Regular Orders:  " << ELT << endl;
	cout << endl;
	
	//overall
	cout << "========== Overall Lead Time Statistics  ==========" << endl;
	map<int, int> LT1; // overall
	for (vector<Order*>::iterator i = AllOrderList.begin(); i != AllOrderList.end(); i++)
	{
		Order* order = *i;
		/*if (order->Type != REGULAR)
		{
			continue;
		}*/

		int lt = order->EndDay - order->StartDay;
		if (LT1.find(lt) == LT1.end())
		{
			LT1[lt] = 0;
		}
		LT1[lt]++;
		
	}

	for (map<int, int>::iterator i = LT1.begin(); i != LT1.end(); i++)
	{
		float o = (float)i->second / (TotalRegualrDemand+TotalUrgentDemand);
		cout << "LT " << i->first << ": " << o << "   (" << i->second << ")" << endl;
		ELT1 += i->first*o;
	}
	cout << "Overall Expected Leadtime :  " << ELT1 << endl;
	cout << "Total Urgent Demands:  " << TotalUrgentDemand << endl;
	cout << "Total Regualr Demands:  " << TotalRegualrDemand << endl;
}
//void StatDetail()
//{
//	cout << "========== Regular Lead Time Statistics ==========" << endl;
//	int total = 0;
//	for (vector<Order*>::iterator i = AllOrderList.begin(); i != AllOrderList.end(); i++)
//	{
//		Order* order = *i;
//		if (order->Type != REGULAR)
//		{
//			continue;
//		}
//		cout << "Type: " << order->Type << " Start: " << order->StartDay << " End: " << order->EndDay << " Exp: " << order->Exp << endl;
//		total++;
//	}
//
//	cout << "========== Regular Lead Time Statistics (End) ==========" << endl;
//}