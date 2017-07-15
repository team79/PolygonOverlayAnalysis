
// MFCApplication3View.cpp : CMFCApplication3View 类的实现
//

#include "stdafx.h"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <cmath>
using namespace std;
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "MFCApplication3.h"
#endif

#include "MFCApplication3Doc.h"
#include "MFCApplication3View.h"

#include <iostream>
#include <cstring>
#include <cstdio>
#include <algorithm>
#include <vector>
#include <cmath>
using namespace std;

const double eps = 1e-10;
const double Pi = acos(-1.0);

int dcmp(double x){
	if (fabs(x) < eps) return 0;
	else return x < 0 ? -1 : 1;
}

class Point{
public:
	double x, y;
	Point(double x = 0, double y = 0) : x(x), y(y){}
	Point& operator=(const Point &b){
		x = b.x;
		y = b.y;
		return *this;
	}

	bool operator<(const Point &b) {
		if (x < b.x){
			return true;
		}
		else if (x == b.x){
			return y < b.y;
		}
		return false;
	}
	bool operator!=(const Point b){
		return !(dcmp(x - b.x) == 0 && dcmp(y - b.y) == 0);
	}
	bool operator==(const Point b){
		return dcmp(x - b.x) == 0 && dcmp(y - b.y) == 0;
	}
};
const Point NOPOINT(1e10, 1e10);
Point operator-(const Point a, const Point b){
	return Point(a.x - b.x, a.y - b.y);
}

class Edge{
public:
	Point LPoint, RPoint;
	int odd;//奇边 ： 1 偶边 ： 0
	int inout;//内边 ： 1 外边： 0 重边 ：2
	void init(){
		if (RPoint < LPoint){
			swap(LPoint, RPoint);
		}
	}
	Edge(){}
	Edge(Point l, Point r) :LPoint(l), RPoint(r){}
	bool operator==(const Edge B){
		return LPoint == B.LPoint && RPoint == B.RPoint;
	}
	bool operator!=(const Edge B){
		return !(LPoint == B.LPoint && RPoint == B.RPoint);
	}
	void output(){
		cout << "(" << LPoint.x << "," << LPoint.y << ")" << "----" << "(" << RPoint.x << "," << RPoint.y << ")" << endl;
		if (odd){
			cout << "奇边" << endl;
		}
		else{
			cout << "偶边" << endl;
		}
		if (inout == 1){
			cout << "内边" << endl;
		}
		else if (inout == 2){
			cout << "重叠边" << endl;
		}
		else{
			cout << "外边" << endl;
		}
	}
};

class TEdge : public Edge{
public:
	int pgn;/* 标记边是属于多边形A、还是多边形B */
	bool visited;/* 标记边是否已访问过, 初始值为false */
	int repet;
	int lpAdjEg, rpAdjEg; /* 指向连接左、右端点的本输入多边形的邻边的指针*/
	int lpotherPgnAdjEg[2], rpotherPgnAdjEg[2]; /* 指向连接左、右端点的另一输入多边形的两邻边的指针*/

	TEdge(){
		memset(lpotherPgnAdjEg, -1, sizeof(lpotherPgnAdjEg));
		memset(rpotherPgnAdjEg, -1, sizeof(rpotherPgnAdjEg));
		visited = false;
	}

	TEdge& operator=(const Edge &e){
		//TEdge ans;
		LPoint = e.LPoint;
		RPoint = e.RPoint;
		odd = e.odd;
		inout = e.inout;
		return *this;
	}

	bool operator==(const TEdge B){
		return LPoint == B.LPoint && RPoint == B.RPoint;
	}
	bool operator!=(const TEdge B){
		return !(LPoint == B.LPoint && RPoint == B.RPoint);
	}
	bool operator==(const Edge B){
		return LPoint == B.LPoint && RPoint == B.RPoint;
	}
	bool operator!=(const Edge B){
		return !(LPoint == B.LPoint && RPoint == B.RPoint);
	}
};

//*************************************************两线段交点
double tCross(const Point &p1, const Point &p2){
	return p1.x*p2.y - p1.y*p2.x;
}

double Cross(const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	return (p2.x - p1.x)*(p4.y - p3.y) - (p2.y - p1.y)*(p4.x - p3.x);
}

double Area(const Point& p1, const Point& p2, const Point& p3)
{
	return Cross(p1, p2, p1, p3);
}

double fArea(const Point& p1, const Point& p2, const Point& p3)
{
	return fabs(Area(p1, p2, p3));
}

bool Meet(const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	return max(min(p1.x, p2.x), min(p3.x, p4.x)) <= min(max(p1.x, p2.x), max(p3.x, p4.x))
		&& max(min(p1.y, p2.y), min(p3.y, p4.y)) <= min(max(p1.y, p2.y), max(p3.y, p4.y))
		&& dcmp(Cross(p3, p2, p3, p4) * Cross(p3, p4, p3, p1)) >= 0
		&& dcmp(Cross(p1, p4, p1, p2) * Cross(p1, p2, p1, p3)) >= 0;
}

Point Inter(const Point& p1, const Point& p2, const Point& p3, const Point& p4)
{
	double s1 = fArea(p1, p2, p3), s2 = fArea(p1, p2, p4);
	return Point((p4.x*s1 + p3.x*s2) / (s1 + s2), (p4.y*s1 + p3.y*s2) / (s1 + s2));
}

double Dot(Point A, Point B){
	return A.x * B.x + A.y * B.y;
}

bool OnSegment(Point p, Point a1, Point a2){
	if (p == a1 || p == a2){
		return true;
	}
	double x1 = min(a1.x, a2.x);
	double x2 = max(a1.x, a2.x);
	double y1 = min(a1.y, a2.y);
	double y2 = max(a1.y, a2.y);
	return dcmp(tCross(a1 - p, a2 - p)) == 0 && p.x >= x1 && p.x <= x2 && p.y >= y1 && p.y <= y2;
}

Point GetCrossPoint(Edge a, Edge b){
	Point a1 = a.LPoint;
	Point a2 = a.RPoint;
	Point b1 = b.LPoint;
	Point b2 = b.RPoint;
	double c1 = tCross(a2 - a1, b1 - a1);
	double c2 = tCross(a2 - a1, b2 - a1);
	double c3 = tCross(b2 - b1, a1 - b1);
	double c4 = tCross(b2 - b1, a2 - b1);
	if (OnSegment(b.LPoint, a.LPoint, a.RPoint)){
		return b.LPoint;
	}
	if (OnSegment(b.RPoint, a.LPoint, a.RPoint)){
		return b.RPoint;
	}
	//	if (dcmp(Cross(a.LPoint, a.RPoint, b.LPoint, b.RPoint)) == 0){
	//		return NOPOINT;
	//	}
	if (dcmp(c1) *dcmp(c2) < 0 && dcmp(c3)*dcmp(c4) < 0){
		return Inter(a.LPoint, a.RPoint, b.LPoint, b.RPoint);
	}
	else{
		return NOPOINT;
	}

}
//************************************************************

class Circle{
public:
	vector<Edge> circle_edge;
	int type;

	Circle(){
		circle_edge.clear();
	}

	void init(){
		for (int i = 0; i < (int)circle_edge.size(); i++){
			circle_edge[i].init();
		}
	}

	/*
	void getSimpleEdge(const MPolygon B){//得到简单边
	vector<Edge> temp1;
	temp1.clear();
	for (int i = 0; i < circle_edge.size(); i++){
	vector<Point> temp2;
	temp2.clear();
	temp2.push_back(circle_edge[i].LPoint);
	temp2.push_back(circle_edge[i].RPoint);
	for (int j = 0; j < B.external_circle.circle_edge.size(); j++){
	Point tempp = GetCrossPoint(circle_edge[i], B.external_circle.circle_edge[j]);
	if (tempp != NOPOINT){
	temp2.push_back(tempp);
	}
	}
	for (int j = 0; j < B.internal_circle.size(); j++){
	for (int k = 0; k < B.internal_circle[j].circle_edge.size(); k++){
	Point tempp = GetCrossPoint(circle_edge[i], B.internal_circle[j].circle_edge[k]);
	if (tempp != NOPOINT){
	temp2.push_back(tempp);
	}
	}
	}
	sort(temp2.cbegin(), temp2.cend());
	for (int j = 0; j < temp2.size() - 1; j++){
	for (int k = j + 1; k < temp2.size(); k++){
	if (temp2[k] != temp2[j]){
	temp1.push_back(Edge(temp2[j], temp2[k]));
	j = k - 1;
	break;
	}
	}
	}
	}
	circle_edge = temp1;
	}
	*/
};

bool PointAboveSegment(Point p, Edge e){
	Point pp(2, 0), ppp(2, 1), pppp(2, 1.5);
	//e.LPoint == pp&& e.RPoint == ppp && 
	//	if (p == pppp ){
	//		int aaa = 1;
	//	}
	if (dcmp(e.LPoint.x - p.x) == 0 && dcmp(e.LPoint.x - e.RPoint.x) == 0 && dcmp(p.y - e.LPoint.y) >= 0 && dcmp(p.y - e.RPoint.y) < 0){
		return true;
	}
	if (dcmp(p.x - e.RPoint.x) >= 0 || dcmp(p.x - e.LPoint.x) < 0){
		return false;
	}
	Point c = p;
	Point a = e.LPoint;
	Point b = e.RPoint;
	return dcmp(c.y - a.y - (c.x - a.x) / (b.x - a.x) * (b.y - a.y)) >= 0;
}

class MPolygon{
public:
	Circle external_circle;//外环
	vector<Circle> internal_circle;//内环

	MPolygon(){
		internal_circle.clear();
	}

	void init(){
		external_circle.init();
		for (int i = 0; i < (int)internal_circle.size(); i++){
			internal_circle[i].init();
		}
	}

	int calc(Point p, const MPolygon B){
		int ans = 0;
		for (int i = 0; i < (int)B.external_circle.circle_edge.size(); i++){
			if (PointAboveSegment(p, B.external_circle.circle_edge[i])){
				ans++;
			}
		}
		for (int j = 0; j < (int)B.internal_circle.size(); j++){
			for (int i = 0; i < (int)B.internal_circle[j].circle_edge.size(); i++){
				if (PointAboveSegment(p, B.internal_circle[j].circle_edge[i])){
					ans++;
				}
			}
		}
		return ans;
	}

	bool finde(Edge e, const MPolygon B){
		for (int i = 0; i < (int)B.external_circle.circle_edge.size(); i++){
			if (e == B.external_circle.circle_edge[i]){
				return true;
			}
		}
		for (int j = 0; j < (int)B.internal_circle.size(); j++){
			for (int i = 0; i < (int)B.internal_circle[j].circle_edge.size(); i++){
				if (e == B.internal_circle[j].circle_edge[i]){
					return true;
				}
			}
		}
		return false;
	}

	void getinout(const MPolygon B){//判断内边还是外边
		for (int i = 0; i < (int)external_circle.circle_edge.size(); i++){
			Point p;
			p.x = (external_circle.circle_edge[i].LPoint.x + external_circle.circle_edge[i].RPoint.x) / 2.0;
			p.y = (external_circle.circle_edge[i].LPoint.y + external_circle.circle_edge[i].RPoint.y) / 2.0;
			if (finde(external_circle.circle_edge[i], B)){
				external_circle.circle_edge[i].inout = 2;
			}
			else if (calc(p, B) & 1){
				external_circle.circle_edge[i].inout = 1;
			}
			else{
				external_circle.circle_edge[i].inout = 0;
			}
		}
		for (int j = 0; j < (int)internal_circle.size(); j++){
			for (int i = 0; i < (int)internal_circle[j].circle_edge.size(); i++){
				Point p;
				p.x = (internal_circle[j].circle_edge[i].LPoint.x + internal_circle[j].circle_edge[i].RPoint.x) / 2.0;
				p.y = (internal_circle[j].circle_edge[i].LPoint.y + internal_circle[j].circle_edge[i].RPoint.y) / 2.0;
				if (finde(internal_circle[j].circle_edge[i], B)){
					internal_circle[j].circle_edge[i].inout = 2;
				}
				else if (calc(p, B) & 1){
					internal_circle[j].circle_edge[i].inout = 1;
				}
				else{
					internal_circle[j].circle_edge[i].inout = 0;
				}
			}
		}
	}

	void getodd(){//判断奇边还是偶边
		for (int i = 0; i < (int)external_circle.circle_edge.size(); i++){
			Point p;
			p.x = (external_circle.circle_edge[i].LPoint.x + external_circle.circle_edge[i].RPoint.x) / 2.0;
			p.y = (external_circle.circle_edge[i].LPoint.y + external_circle.circle_edge[i].RPoint.y) / 2.0;
			if (calc(p, *this) & 1){
				external_circle.circle_edge[i].odd = 1;
			}
			else{
				external_circle.circle_edge[i].odd = 0;
			}
		}
		for (int j = 0; j < (int)internal_circle.size(); j++){
			for (int i = 0; i < (int)internal_circle[j].circle_edge.size(); i++){
				Point p;
				p.x = (internal_circle[j].circle_edge[i].LPoint.x + internal_circle[j].circle_edge[i].RPoint.x) / 2.0;
				p.y = (internal_circle[j].circle_edge[i].LPoint.y + internal_circle[j].circle_edge[i].RPoint.y) / 2.0;
				if (calc(p, *this) & 1){
					internal_circle[j].circle_edge[i].odd = 1;
				}
				else{
					internal_circle[j].circle_edge[i].odd = 0;
				}
			}
		}
	}

	void getSimpleEdge(const MPolygon B){//得到简单边
		vector<Edge> temp1;
		temp1.clear();

		//处理外环
		for (int i = 0; i < (int)external_circle.circle_edge.size(); i++){
			vector<Point> temp2;
			temp2.clear();
			temp2.push_back(external_circle.circle_edge[i].LPoint);
			temp2.push_back(external_circle.circle_edge[i].RPoint);
			for (int j = 0; j < (int)B.external_circle.circle_edge.size(); j++){
				Edge a = external_circle.circle_edge[i];
				Edge b = B.external_circle.circle_edge[j];
				Point tempp = GetCrossPoint(external_circle.circle_edge[i], B.external_circle.circle_edge[j]);
				if (tempp != NOPOINT){
					temp2.push_back(tempp);
				}
			}
			for (int j = 0; j < (int)B.internal_circle.size(); j++){
				for (int k = 0; k < (int)B.internal_circle[j].circle_edge.size(); k++){
					Point tempp = GetCrossPoint(external_circle.circle_edge[i], B.internal_circle[j].circle_edge[k]);
					if (tempp != NOPOINT){
						temp2.push_back(tempp);
					}
				}
			}
			sort(temp2.begin(), temp2.end());
			for (int j = 0; j < (int)temp2.size() - 1; j++){
				for (int k = j + 1; k < (int)temp2.size(); k++){
					if (temp2[k] != temp2[j]){
						temp1.push_back(Edge(temp2[j], temp2[k]));
						j = k - 1;
						break;
					}
				}
			}
		}
		external_circle.circle_edge = temp1;


		//处理内环
		for (int l = 0; l < (int)internal_circle.size(); l++){
			vector<Edge> temp1;
			temp1.clear();
			for (int i = 0; i <(int)internal_circle[l].circle_edge.size(); i++){
				vector<Point> temp2;
				temp2.clear();
				temp2.push_back(internal_circle[l].circle_edge[i].LPoint);
				temp2.push_back(internal_circle[l].circle_edge[i].RPoint);
				for (int j = 0; j < (int)B.external_circle.circle_edge.size(); j++){
					Point tempp = GetCrossPoint(internal_circle[l].circle_edge[i], B.external_circle.circle_edge[j]);
					if (tempp != NOPOINT){
						temp2.push_back(tempp);
					}
				}
				for (int j = 0; j < (int)B.internal_circle.size(); j++){
					for (int k = 0; k < (int)B.internal_circle[j].circle_edge.size(); k++){
						Point tempp = GetCrossPoint(internal_circle[l].circle_edge[i], B.internal_circle[j].circle_edge[k]);
						if (tempp != NOPOINT){
							temp2.push_back(tempp);
						}
					}
				}
				sort(temp2.begin(), temp2.end());
				for (int j = 0; j < (int)temp2.size() - 1; j++){
					for (int k = j + 1; k < (int)temp2.size(); k++){
						if (temp2[k] != temp2[j]){
							temp1.push_back(Edge(temp2[j], temp2[k]));
							j = k - 1;
							break;
						}
					}
				}
			}
			internal_circle[l].circle_edge = temp1;
		}
	}

};

int solve_and(MPolygon p1, MPolygon p2, CClientDC *cdc){
	p1.init();
	p2.init();
	p1.getSimpleEdge(p2);
	p2.getSimpleEdge(p1);
	p1.getinout(p2);
	p2.getinout(p1);
	p1.getodd();
	p2.getodd();

	vector<TEdge> edge;
	edge.clear();

	for (int i = 0; i < p1.external_circle.circle_edge.size(); i++){
		TEdge temp;
		temp = p1.external_circle.circle_edge[i];
		temp.pgn = 1;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p1.internal_circle.size(); j++){
		for (int i = 0; i < p1.internal_circle[j].circle_edge.size(); i++){
			TEdge temp;
			temp = p1.internal_circle[j].circle_edge[i];
			temp.pgn = 1;
			temp.visited = false;
			edge.push_back(temp);
		}
	}
	for (int i = 0; i < p2.external_circle.circle_edge.size(); i++){
		TEdge temp;
		temp = p2.external_circle.circle_edge[i];
		temp.pgn = 2;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p2.internal_circle.size(); j++){
		for (int i = 0; i < p2.internal_circle[j].circle_edge.size(); i++){
			TEdge temp;
			temp = p2.internal_circle[j].circle_edge[i];
			temp.pgn = 2;
			temp.visited = false;
			edge.push_back(temp);
		}
	}

	for (int i = 0; i < edge.size(); i++){
		for (int j = 0; j < edge.size(); j++){
			if (i == j)	continue;
			if (edge[j].pgn == edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint)){
				edge[i].lpAdjEg = j;
			}
			if (edge[j].pgn == edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint)){
				edge[i].rpAdjEg = j;
			}
		}
		int t1 = 0, t2 = 0;
		for (int j = 0; j < edge.size(); j++){
			if (i == j)	continue;
			if (edge[j].pgn != edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint)){
				edge[i].lpotherPgnAdjEg[t1++] = j;
			}
			if (edge[j].pgn != edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint)){
				edge[i].rpotherPgnAdjEg[t2++] = j;
			}
		}
		if (edge[i].inout == 2){
			for (int j = 0; j < edge.size(); j++){
				if (i == j)	continue;
				if (edge[j].pgn != edge[i].pgn && edge[j] == edge[i]){
					edge[i].repet = j;
				}
			}
		}
	}

//	for (int i = 0; i < edge.size(); i++){
//		cout << i << " :  ";
//		edge[i].output();
//		cout << edge[i].lpAdjEg << " " << edge[i].rpAdjEg << " " << edge[i].lpotherPgnAdjEg[0] << " " << edge[i].lpotherPgnAdjEg[1] << " " << edge[i].rpotherPgnAdjEg[0] << " " << edge[i].rpotherPgnAdjEg[1] << endl;
//	}


	for (int i = 0; i < edge.size(); i++){
		vector<Point> ansv;
		ansv.clear();
		if (!edge[i].visited && edge[i].inout == 1){
			Point P;
			int e = i;
			do{
				edge[e].visited = true;
				if (edge[e].odd == 1){
					ansv.push_back(edge[e].LPoint);
					P = edge[e].RPoint;
					if (edge[e].rpotherPgnAdjEg[0] != -1
						&& edge[edge[e].rpotherPgnAdjEg[0]].inout == 1
						&& ((edge[edge[e].rpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].rpotherPgnAdjEg[0]].LPoint == P)
						|| (edge[edge[e].rpotherPgnAdjEg[0]].odd == 0 && edge[edge[e].rpotherPgnAdjEg[0]].RPoint == P))){
						e = edge[e].rpotherPgnAdjEg[0];
					}
					else if (edge[e].rpotherPgnAdjEg[1] != -1
						&& edge[edge[e].rpotherPgnAdjEg[1]].inout == 1
						&& ((edge[edge[e].rpotherPgnAdjEg[1]].odd == 1 && edge[edge[e].rpotherPgnAdjEg[1]].LPoint == P)
						|| (edge[edge[e].rpotherPgnAdjEg[1]].odd == 0 && edge[edge[e].rpotherPgnAdjEg[1]].RPoint == P))){
						e = edge[e].rpotherPgnAdjEg[1];
					}
					else{
						e = edge[e].rpAdjEg;
					}
				}
				else{
					ansv.push_back(edge[e].RPoint);
					P = edge[e].LPoint;
					if (edge[e].lpotherPgnAdjEg[0] != -1
						&& edge[edge[e].lpotherPgnAdjEg[0]].inout == 1
						&& ((edge[edge[e].lpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].lpotherPgnAdjEg[0]].LPoint == P)
						|| (edge[edge[e].lpotherPgnAdjEg[0]].odd == 0 && edge[edge[e].lpotherPgnAdjEg[0]].RPoint == P))){
						e = edge[e].lpotherPgnAdjEg[0];
					}
					else if (edge[e].lpotherPgnAdjEg[1] != -1
						&& edge[edge[e].lpotherPgnAdjEg[1]].inout == 1
						&& ((edge[edge[e].lpotherPgnAdjEg[1]].odd == 1 && edge[edge[e].lpotherPgnAdjEg[1]].LPoint == P)
						|| (edge[edge[e].lpotherPgnAdjEg[1]].odd == 0 && edge[edge[e].lpotherPgnAdjEg[1]].RPoint == P))){
						e = edge[e].lpotherPgnAdjEg[1];
					}
					else{
						e = edge[e].lpAdjEg;
					}
				}
			} while (ansv[0] != P);
			CPen pen(PS_SOLID, 5, RGB(0, 0, 0));
			CPen *pOldPen = cdc->SelectObject(&pen);
			for (int j = 0; j < ansv.size() - 1; j++){
				CPoint t1, t2;
				t1.x = ansv[j].x;
				t1.y = ansv[j].y;
				t2.x = ansv[j+1].x;
				t2.y = ansv[j+1].y;
				cdc->MoveTo(t1);
				cdc->LineTo(t2);
			//	cout << "(" << ansv[j].x << "," << ansv[j].y << ")" << endl;
			}
			CPoint t1, t2;
			t1.x = ansv[0].x;
			t1.y = ansv[0].y;
			t2.x = ansv[ansv.size()-1].x;
			t2.y = ansv[ansv.size() - 1].y;
			cdc->MoveTo(t1);
			cdc->LineTo(t2);
		}
	}
	return 0;
}

int solve_or(MPolygon p1, MPolygon p2, CClientDC *cdc){
	p1.init();
	p2.init();
	p1.getSimpleEdge(p2);
	p2.getSimpleEdge(p1);
	p1.getinout(p2);
	p2.getinout(p1);
	p1.getodd();
	p2.getodd();

	vector<TEdge> edge;
	edge.clear();

	for (int i = 0; i < p1.external_circle.circle_edge.size(); i++){
		TEdge temp;
		temp = p1.external_circle.circle_edge[i];
		temp.pgn = 1;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p1.internal_circle.size(); j++){
		for (int i = 0; i < p1.internal_circle[j].circle_edge.size(); i++){
			TEdge temp;
			temp = p1.internal_circle[j].circle_edge[i];
			temp.pgn = 1;
			temp.visited = false;
			edge.push_back(temp);
		}
	}
	for (int i = 0; i < p2.external_circle.circle_edge.size(); i++){
		TEdge temp;
		temp = p2.external_circle.circle_edge[i];
		temp.pgn = 2;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p2.internal_circle.size(); j++){
		for (int i = 0; i < p2.internal_circle[j].circle_edge.size(); i++){
			TEdge temp;
			temp = p2.internal_circle[j].circle_edge[i];
			temp.pgn = 2;
			temp.visited = false;
			edge.push_back(temp);
		}
	}

	for (int i = 0; i < edge.size(); i++){
		for (int j = 0; j < edge.size(); j++){
			if (i == j)	continue;
			if (edge[j].pgn == edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint)){
				edge[i].lpAdjEg = j;
			}
			if (edge[j].pgn == edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint)){
				edge[i].rpAdjEg = j;
			}
		}
		int t1 = 0, t2 = 0;
		for (int j = 0; j < edge.size(); j++){
			if (i == j)	continue;
			if (edge[j].pgn != edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint)){
				edge[i].lpotherPgnAdjEg[t1++] = j;
			}
			if (edge[j].pgn != edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint)){
				edge[i].rpotherPgnAdjEg[t2++] = j;
			}
		}
		if (edge[i].inout == 2){
			for (int j = 0; j < edge.size(); j++){
				if (i == j)	continue;
				if (edge[j].pgn != edge[i].pgn && edge[j] == edge[i]){
					edge[i].repet = j;
				}
			}
		}
	}

	//	for (int i = 0; i < edge.size(); i++){
	//		cout << i << " :  ";
	//		edge[i].output();
	//		cout << edge[i].lpAdjEg << " " << edge[i].rpAdjEg << " " << edge[i].lpotherPgnAdjEg[0] << " " << edge[i].lpotherPgnAdjEg[1] << " " << edge[i].rpotherPgnAdjEg[0] << " " << edge[i].rpotherPgnAdjEg[1] << endl;
	//	}


	for (int i = 0; i < edge.size(); i++){
		vector<Point> ansv;
		ansv.clear();
		if (!edge[i].visited && edge[i].inout == 0){
			Point P;
			int e = i;
			do{
				edge[e].visited = true;
				if (edge[e].odd == 1){
					ansv.push_back(edge[e].LPoint);
					P = edge[e].RPoint;
					if (edge[edge[e].rpAdjEg].inout == 0){
						e = edge[e].rpAdjEg;
					}
					else if (edge[e].rpotherPgnAdjEg[0] != -1
						&& edge[edge[e].rpotherPgnAdjEg[0]].inout == 0){
						e = edge[e].rpotherPgnAdjEg[0];
					}
					else if (edge[e].rpotherPgnAdjEg[1] != -1
						&& edge[edge[e].rpotherPgnAdjEg[1]].inout == 0){
						e = edge[e].rpotherPgnAdjEg[1];
					}
					else{
						e = edge[e].rpAdjEg;
					}
				}
				else{
					ansv.push_back(edge[e].RPoint);
					P = edge[e].LPoint;
					if (edge[edge[e].lpAdjEg].inout == 0){
						e = edge[e].lpAdjEg;
					}
					else if (edge[e].lpotherPgnAdjEg[0] != -1
						&& edge[edge[e].lpotherPgnAdjEg[0]].inout == 0){
						e = edge[e].lpotherPgnAdjEg[0];
					}
					else if (edge[e].lpotherPgnAdjEg[1] != -1
						&& edge[edge[e].lpotherPgnAdjEg[1]].inout == 0){
						e = edge[e].lpotherPgnAdjEg[1];
					}
					else{
						e = edge[e].lpAdjEg;
					}
				}
			} while (ansv[0] != P);
			CPen pen(PS_SOLID, 5, RGB(100, 100, 100));
			CPen *pOldPen = cdc->SelectObject(&pen);
			for (int j = 0; j < ansv.size() - 1; j++){
				CPoint t1, t2;
				t1.x = ansv[j].x;
				t1.y = ansv[j].y;
				t2.x = ansv[j + 1].x;
				t2.y = ansv[j + 1].y;
				cdc->MoveTo(t1);
				cdc->LineTo(t2);
				//	cout << "(" << ansv[j].x << "," << ansv[j].y << ")" << endl;
			}
			CPoint t1, t2;
			t1.x = ansv[0].x;
			t1.y = ansv[0].y;
			t2.x = ansv[ansv.size() - 1].x;
			t2.y = ansv[ansv.size() - 1].y;
			cdc->MoveTo(t1);
			cdc->LineTo(t2);
		}
	}
	return 0;
}

int solve_sub(MPolygon p1, MPolygon p2, CClientDC *cdc){
	p1.init();
	p2.init();
	p1.getSimpleEdge(p2);
	p2.getSimpleEdge(p1);
	p1.getinout(p2);
	p2.getinout(p1);
	p1.getodd();
	p2.getodd();

	vector<TEdge> edge;
	edge.clear();

	for (int i = 0; i < p1.external_circle.circle_edge.size(); i++){
		TEdge temp;
		temp = p1.external_circle.circle_edge[i];
		temp.pgn = 1;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p1.internal_circle.size(); j++){
		for (int i = 0; i < p1.internal_circle[j].circle_edge.size(); i++){
			TEdge temp;
			temp = p1.internal_circle[j].circle_edge[i];
			temp.pgn = 1;
			temp.visited = false;
			edge.push_back(temp);
		}
	}
	for (int i = 0; i < p2.external_circle.circle_edge.size(); i++){
		TEdge temp;
		temp = p2.external_circle.circle_edge[i];
		temp.pgn = 2;
		temp.visited = false;
		edge.push_back(temp);
	}
	for (int j = 0; j < p2.internal_circle.size(); j++){
		for (int i = 0; i < p2.internal_circle[j].circle_edge.size(); i++){
			TEdge temp;
			temp = p2.internal_circle[j].circle_edge[i];
			temp.pgn = 2;
			temp.visited = false;
			edge.push_back(temp);
		}
	}

	for (int i = 0; i < edge.size(); i++){
		for (int j = 0; j < edge.size(); j++){
			if (i == j)	continue;
			if (edge[j].pgn == edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint)){
				edge[i].lpAdjEg = j;
			}
			if (edge[j].pgn == edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint)){
				edge[i].rpAdjEg = j;
			}
		}
		int t1 = 0, t2 = 0;
		for (int j = 0; j < edge.size(); j++){
			if (i == j)	continue;
			if (edge[j].pgn != edge[i].pgn && (edge[i].LPoint == edge[j].LPoint || edge[i].LPoint == edge[j].RPoint)){
				edge[i].lpotherPgnAdjEg[t1++] = j;
			}
			if (edge[j].pgn != edge[i].pgn && (edge[i].RPoint == edge[j].LPoint || edge[i].RPoint == edge[j].RPoint)){
				edge[i].rpotherPgnAdjEg[t2++] = j;
			}
		}
		if (edge[i].inout == 2){
			for (int j = 0; j < edge.size(); j++){
				if (i == j)	continue;
				if (edge[j].pgn != edge[i].pgn && edge[j] == edge[i]){
					edge[i].repet = j;
				}
			}
		}
	}

	//	for (int i = 0; i < edge.size(); i++){
	//		cout << i << " :  ";
	//		edge[i].output();
	//		cout << edge[i].lpAdjEg << " " << edge[i].rpAdjEg << " " << edge[i].lpotherPgnAdjEg[0] << " " << edge[i].lpotherPgnAdjEg[1] << " " << edge[i].rpotherPgnAdjEg[0] << " " << edge[i].rpotherPgnAdjEg[1] << endl;
	//	}


	for (int i = 0; i < edge.size(); i++){
		vector<Point> ansv;
		ansv.clear();
		if (!edge[i].visited && ((edge[i].inout == 0 && edge[i].pgn == 1) || (edge[i].inout == 1 && edge[i].pgn == 2))){
			Point P;
			int e = i;
			do{
				edge[e].visited = true;
				if ((edge[e].odd == 1 && edge[e].pgn == 1) || (edge[e].odd == 0 && edge[e].pgn == 2)){
					ansv.push_back(edge[e].LPoint);
					P = edge[e].RPoint;
					if (edge[e].pgn == 1
						&& edge[e].rpotherPgnAdjEg[0] != -1
						&& edge[edge[e].rpotherPgnAdjEg[0]].inout == 1
						&& ((edge[edge[e].rpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].rpotherPgnAdjEg[0]].RPoint == P)
						|| (edge[edge[e].rpotherPgnAdjEg[0]].odd == 0 && edge[edge[e].rpotherPgnAdjEg[0]].LPoint == P))){
						e = edge[e].rpotherPgnAdjEg[0];
					}
					else if (edge[e].pgn == 1
						&& edge[e].rpotherPgnAdjEg[1] != -1
						&& edge[edge[e].rpotherPgnAdjEg[1]].inout == 1
						&& ((edge[edge[e].rpotherPgnAdjEg[1]].odd == 1 && edge[edge[e].rpotherPgnAdjEg[1]].RPoint == P)
						|| (edge[edge[e].rpotherPgnAdjEg[1]].odd == 0 && edge[edge[e].rpotherPgnAdjEg[1]].LPoint == P))){
						e = edge[e].rpotherPgnAdjEg[1];
					}
					else if (edge[e].pgn == 2
						&& edge[e].rpotherPgnAdjEg[0] != -1
						&& edge[edge[e].rpotherPgnAdjEg[0]].inout == 0
						&& ((edge[edge[e].rpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].rpotherPgnAdjEg[0]].LPoint == P)
						|| (edge[edge[e].rpotherPgnAdjEg[0]].odd == 0 && edge[edge[e].rpotherPgnAdjEg[0]].RPoint == P))){
						e = edge[e].rpotherPgnAdjEg[0];
					}
					else if (edge[e].pgn == 2
						&& edge[e].rpotherPgnAdjEg[1] != -1
						&& edge[edge[e].rpotherPgnAdjEg[1]].inout == 0
						&& ((edge[edge[e].rpotherPgnAdjEg[1]].odd == 1 && edge[edge[e].rpotherPgnAdjEg[1]].LPoint == P)
						|| (edge[edge[e].rpotherPgnAdjEg[1]].odd == 0 && edge[edge[e].rpotherPgnAdjEg[1]].RPoint == P))){
						e = edge[e].rpotherPgnAdjEg[1];
					}
					else{
						e = edge[e].rpAdjEg;
					}
				}
				else{
					ansv.push_back(edge[e].RPoint);
					P = edge[e].LPoint;
					if (edge[e].pgn == 1
						&& edge[e].lpotherPgnAdjEg[0] != -1
						&& edge[edge[e].lpotherPgnAdjEg[0]].inout == 1
						&& ((edge[edge[e].lpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].lpotherPgnAdjEg[0]].RPoint == P)
						|| (edge[edge[e].lpotherPgnAdjEg[0]].odd == 0 && edge[edge[e].lpotherPgnAdjEg[0]].LPoint == P))){
						e = edge[e].lpotherPgnAdjEg[0];
					}
					else if (edge[e].pgn == 1
						&& edge[e].lpotherPgnAdjEg[1] != -1
						&& edge[edge[e].lpotherPgnAdjEg[1]].inout == 1
						&& ((edge[edge[e].lpotherPgnAdjEg[1]].odd == 1 && edge[edge[e].lpotherPgnAdjEg[1]].RPoint == P)
						|| (edge[edge[e].lpotherPgnAdjEg[1]].odd == 0 && edge[edge[e].lpotherPgnAdjEg[1]].LPoint == P))){
						e = edge[e].lpotherPgnAdjEg[1];
					}
					else if (edge[e].pgn == 2
						&& edge[e].lpotherPgnAdjEg[0] != -1
						&& edge[edge[e].lpotherPgnAdjEg[0]].inout == 0
						&& ((edge[edge[e].lpotherPgnAdjEg[0]].odd == 1 && edge[edge[e].lpotherPgnAdjEg[0]].LPoint == P)
						|| (edge[edge[e].lpotherPgnAdjEg[0]].odd == 0 && edge[edge[e].lpotherPgnAdjEg[0]].RPoint == P))){
						e = edge[e].lpotherPgnAdjEg[0];
					}
					else if (edge[e].pgn == 2
						&& edge[e].lpotherPgnAdjEg[1] != -1
						&& edge[edge[e].lpotherPgnAdjEg[1]].inout == 0
						&& ((edge[edge[e].lpotherPgnAdjEg[1]].odd == 1 && edge[edge[e].lpotherPgnAdjEg[1]].LPoint == P)
						|| (edge[edge[e].lpotherPgnAdjEg[1]].odd == 0 && edge[edge[e].lpotherPgnAdjEg[1]].RPoint == P))){
						e = edge[e].lpotherPgnAdjEg[1];
					}
					else{
						e = edge[e].lpAdjEg;
					}
				}
			} while (ansv[0] != P);
			CPen pen(PS_SOLID, 5, RGB(0, 0, 0));
			CPen *pOldPen = cdc->SelectObject(&pen);
			for (int j = 0; j < ansv.size() - 1; j++){
				CPoint t1, t2;
				t1.x = ansv[j].x;
				t1.y = ansv[j].y;
				t2.x = ansv[j + 1].x;
				t2.y = ansv[j + 1].y;
				cdc->MoveTo(t1);
				cdc->LineTo(t2);
				//	cout << "(" << ansv[j].x << "," << ansv[j].y << ")" << endl;
			}
			CPoint t1, t2;
			t1.x = ansv[0].x;
			t1.y = ansv[0].y;
			t2.x = ansv[ansv.size() - 1].x;
			t2.y = ansv[ansv.size() - 1].y;
			cdc->MoveTo(t1);
			cdc->LineTo(t2);
		}
	}
	return 0;
}

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// CMFCApplication3View

IMPLEMENT_DYNCREATE(CMFCApplication3View, CView)

BEGIN_MESSAGE_MAP(CMFCApplication3View, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CView::OnFilePrintPreview)
	ON_COMMAND(ID_A, &CMFCApplication3View::OnA)
//	ON_WM_MBUTTONDOWN()
	ON_WM_LBUTTONDOWN()
	ON_WM_RBUTTONDOWN()
	ON_COMMAND(ID_B, &CMFCApplication3View::OnB)
	ON_COMMAND(ID_AANDB, &CMFCApplication3View::OnAandb)
	ON_COMMAND(ID_AORB, &CMFCApplication3View::OnAorb)
	ON_COMMAND(ID_ASUBB, &CMFCApplication3View::OnAsubb)
END_MESSAGE_MAP()

// CMFCApplication3View 构造/析构



CMFCApplication3View::CMFCApplication3View()
{
	// TODO:  在此处添加构造代码

}

CMFCApplication3View::~CMFCApplication3View()
{
}

BOOL CMFCApplication3View::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO:  在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CMFCApplication3View 绘制

void CMFCApplication3View::OnDraw(CDC* /*pDC*/)
{
	CMFCApplication3Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO:  在此处为本机数据添加绘制代码
}


// CMFCApplication3View 打印

BOOL CMFCApplication3View::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CMFCApplication3View::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  添加额外的打印前进行的初始化过程
}

void CMFCApplication3View::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO:  添加打印后进行的清理过程
}


// CMFCApplication3View 诊断

#ifdef _DEBUG
void CMFCApplication3View::AssertValid() const
{
	CView::AssertValid();
}

void CMFCApplication3View::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CMFCApplication3Doc* CMFCApplication3View::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CMFCApplication3Doc)));
	return (CMFCApplication3Doc*)m_pDocument;
}
#endif //_DEBUG


// CMFCApplication3View 消息处理程序


void CMFCApplication3View::OnA()
{
	RedrawWindow();
	nowp = 0;
	memset(epsz, 0, sizeof(epsz));
	memset(ipsz1, 0, sizeof(ipsz1));
	memset(ipsz2, 0, sizeof(ipsz2));
	memset(flag, 0, sizeof(flag));
	ftt = -1;
	// TODO:  在此添加命令处理程序代码
}


//void CMFCApplication3View::OnMButtonDown(UINT nFlags, CPoint point)
//{
//	// TODO:  在此添加消息处理程序代码和/或调用默认值
//	
//	CView::OnMButtonDown(nFlags, point);
//}


void CMFCApplication3View::OnLButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (flag[nowp] == 0){
		ep[nowp][epsz[nowp]++] = point;
	}
	else{
		ip[nowp][ipsz1[nowp]][ipsz2[nowp][ipsz1[nowp]]++] = point;
	}
	if (ftt == -1){
		ftt = 1;
		ft = point;
	}
	else{
		if (nowp == 0){
			CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
			CClientDC cdc(this);
			CPen *pOldPen = cdc.SelectObject(&pen);
			cdc.MoveTo(pre);
			cdc.LineTo(point);
		}
		else{
			CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
			CClientDC cdc(this);
			CPen *pOldPen = cdc.SelectObject(&pen);
			cdc.MoveTo(pre);
			cdc.LineTo(point);
		}
		
	}
	pre = point;
//	MessageBox(_T("31231"));
	CView::OnLButtonDown(nFlags, point);
}


void CMFCApplication3View::OnRButtonDown(UINT nFlags, CPoint point)
{
	// TODO:  在此添加消息处理程序代码和/或调用默认值
	if (nowp == 0){
		CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
		CClientDC cdc(this);
		CPen *pOldPen = cdc.SelectObject(&pen);
		cdc.MoveTo(pre);
		cdc.LineTo(ft);
	}
	else{
		CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
		CClientDC cdc(this);
		CPen *pOldPen = cdc.SelectObject(&pen);
		cdc.MoveTo(pre);
		cdc.LineTo(ft);
	}
	ftt = -1;
	if (flag[nowp] == 1){
		ipsz1[nowp]++;
	}
	flag[nowp] = 1;
	CView::OnRButtonDown(nFlags, point);
}


void CMFCApplication3View::OnB()
{
	// TODO:  在此添加命令处理程序代码
	nowp = 1;
	ftt = -1;
}


void CMFCApplication3View::OnAandb()
{
	MPolygon p1, p2;
	Edge temp;
	for (int i = 0; i < epsz[0] - 1; i++){
		
		temp.LPoint.x = ep[0][i].x;
		temp.LPoint.y = ep[0][i].y;
		temp.RPoint.x = ep[0][i+1].x;
		temp.RPoint.y = ep[0][i+1].y;
		p1.external_circle.circle_edge.push_back(temp);
	}
	temp.LPoint.x = ep[0][0].x;
	temp.LPoint.y = ep[0][0].y;
	temp.RPoint.x = ep[0][epsz[0]-1].x;
	temp.RPoint.y = ep[0][epsz[0]-1].y;
	p1.external_circle.circle_edge.push_back(temp);
	for (int i = 0; i < ipsz1[0]; i++){
		Circle tc;
		p1.internal_circle.push_back(tc);
		for (int j = 0; j < ipsz2[0][i] - 1; j++){
			temp.LPoint.x = ip[0][i][j].x;
			temp.LPoint.y = ip[0][i][j].y;
			temp.RPoint.x = ip[0][i][j+1].x;
			temp.RPoint.y = ip[0][i][j+1].y;
			p1.internal_circle[i].circle_edge.push_back(temp);
		}
		temp.LPoint.x = ip[0][i][0].x;
		temp.LPoint.y = ip[0][i][0].y;
		temp.RPoint.x = ip[0][i][ipsz2[0][i]-1].x;
		temp.RPoint.y = ip[0][i][ipsz2[0][i]-1].y;
		p1.internal_circle[i].circle_edge.push_back(temp);
	}
	for (int i = 0; i < epsz[1] - 1; i++){
		temp.LPoint.x = ep[1][i].x;
		temp.LPoint.y = ep[1][i].y;
		temp.RPoint.x = ep[1][i + 1].x;
		temp.RPoint.y = ep[1][i + 1].y;
		p2.external_circle.circle_edge.push_back(temp);
	}
	temp.LPoint.x = ep[1][0].x;
	temp.LPoint.y = ep[1][0].y;
	temp.RPoint.x = ep[1][epsz[1] - 1].x;
	temp.RPoint.y = ep[1][epsz[1] - 1].y;
	p2.external_circle.circle_edge.push_back(temp);
	for (int i = 0; i < ipsz1[1]; i++){
		Circle tc;
		p2.internal_circle.push_back(tc);
		for (int j = 0; j < ipsz2[1][i] - 1; j++){
			temp.LPoint.x = ip[1][i][j].x;
			temp.LPoint.y = ip[1][i][j].y;
			temp.RPoint.x = ip[1][i][j + 1].x;
			temp.RPoint.y = ip[1][i][j + 1].y;
			p2.internal_circle[i].circle_edge.push_back(temp);
		}
		temp.LPoint.x = ip[1][i][0].x;
		temp.LPoint.y = ip[1][i][0].y;
		temp.RPoint.x = ip[1][i][ipsz2[1][i] - 1].x;
		temp.RPoint.y = ip[1][i][ipsz2[1][i] - 1].y;
		p2.internal_circle[i].circle_edge.push_back(temp);
	}
	RedrawWindow();
	CClientDC cdc(this);
	for (int i = 0; i < p1.external_circle.circle_edge.size(); i++){
		CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
		CPen *pOldPen = cdc.SelectObject(&pen);
		CPoint t1, t2;
		t1.x = p1.external_circle.circle_edge[i].LPoint.x;
		t1.y = p1.external_circle.circle_edge[i].LPoint.y;
		t2.x = p1.external_circle.circle_edge[i].RPoint.x;
		t2.y = p1.external_circle.circle_edge[i].RPoint.y;
		cdc.MoveTo(t1);
		cdc.LineTo(t2);
	}
	for (int j = 0; j < p1.internal_circle.size(); j++){
		for (int i = 0; i < p1.internal_circle[j].circle_edge.size(); i++){
			CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
			CPen *pOldPen = cdc.SelectObject(&pen);
			CPoint t1, t2;
			t1.x = p1.internal_circle[j].circle_edge[i].LPoint.x;
			t1.y = p1.internal_circle[j].circle_edge[i].LPoint.y;
			t2.x = p1.internal_circle[j].circle_edge[i].RPoint.x;
			t2.y = p1.internal_circle[j].circle_edge[i].RPoint.y;
			cdc.MoveTo(t1);
			cdc.LineTo(t2);
		}
	}
	for (int i = 0; i < p2.external_circle.circle_edge.size(); i++){
		CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
		CPen *pOldPen = cdc.SelectObject(&pen);
		CPoint t1, t2;
		t1.x = p2.external_circle.circle_edge[i].LPoint.x;
		t1.y = p2.external_circle.circle_edge[i].LPoint.y;
		t2.x = p2.external_circle.circle_edge[i].RPoint.x;
		t2.y = p2.external_circle.circle_edge[i].RPoint.y;
		cdc.MoveTo(t1);
		cdc.LineTo(t2);
	}
	for (int j = 0; j < p2.internal_circle.size(); j++){
		for (int i = 0; i < p2.internal_circle[j].circle_edge.size(); i++){
			CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
			CPen *pOldPen = cdc.SelectObject(&pen);
			CPoint t1, t2;
			t1.x = p2.internal_circle[j].circle_edge[i].LPoint.x;
			t1.y = p2.internal_circle[j].circle_edge[i].LPoint.y;
			t2.x = p2.internal_circle[j].circle_edge[i].RPoint.x;
			t2.y = p2.internal_circle[j].circle_edge[i].RPoint.y;
			cdc.MoveTo(t1);
			cdc.LineTo(t2);
		}
	}
	solve_and(p1, p2, &cdc);
	// TODO:  在此添加命令处理程序代码
}

void CMFCApplication3View::OnAorb()
{
	// TODO:  在此添加命令处理程序代码
	MPolygon p1, p2;
	Edge temp;
	for (int i = 0; i < epsz[0] - 1; i++){

		temp.LPoint.x = ep[0][i].x;
		temp.LPoint.y = ep[0][i].y;
		temp.RPoint.x = ep[0][i + 1].x;
		temp.RPoint.y = ep[0][i + 1].y;
		p1.external_circle.circle_edge.push_back(temp);
	}
	temp.LPoint.x = ep[0][0].x;
	temp.LPoint.y = ep[0][0].y;
	temp.RPoint.x = ep[0][epsz[0] - 1].x;
	temp.RPoint.y = ep[0][epsz[0] - 1].y;
	p1.external_circle.circle_edge.push_back(temp);
	for (int i = 0; i < ipsz1[0]; i++){
		Circle tc;
		p1.internal_circle.push_back(tc);
		for (int j = 0; j < ipsz2[0][i] - 1; j++){
			temp.LPoint.x = ip[0][i][j].x;
			temp.LPoint.y = ip[0][i][j].y;
			temp.RPoint.x = ip[0][i][j + 1].x;
			temp.RPoint.y = ip[0][i][j + 1].y;
			p1.internal_circle[i].circle_edge.push_back(temp);
		}
		temp.LPoint.x = ip[0][i][0].x;
		temp.LPoint.y = ip[0][i][0].y;
		temp.RPoint.x = ip[0][i][ipsz2[0][i] - 1].x;
		temp.RPoint.y = ip[0][i][ipsz2[0][i] - 1].y;
		p1.internal_circle[i].circle_edge.push_back(temp);
	}
	for (int i = 0; i < epsz[1] - 1; i++){
		temp.LPoint.x = ep[1][i].x;
		temp.LPoint.y = ep[1][i].y;
		temp.RPoint.x = ep[1][i + 1].x;
		temp.RPoint.y = ep[1][i + 1].y;
		p2.external_circle.circle_edge.push_back(temp);
	}
	temp.LPoint.x = ep[1][0].x;
	temp.LPoint.y = ep[1][0].y;
	temp.RPoint.x = ep[1][epsz[1] - 1].x;
	temp.RPoint.y = ep[1][epsz[1] - 1].y;
	p2.external_circle.circle_edge.push_back(temp);
	for (int i = 0; i < ipsz1[1]; i++){
		Circle tc;
		p2.internal_circle.push_back(tc);
		for (int j = 0; j < ipsz2[1][i] - 1; j++){
			temp.LPoint.x = ip[1][i][j].x;
			temp.LPoint.y = ip[1][i][j].y;
			temp.RPoint.x = ip[1][i][j + 1].x;
			temp.RPoint.y = ip[1][i][j + 1].y;
			p2.internal_circle[i].circle_edge.push_back(temp);
		}
		temp.LPoint.x = ip[1][i][0].x;
		temp.LPoint.y = ip[1][i][0].y;
		temp.RPoint.x = ip[1][i][ipsz2[1][i] - 1].x;
		temp.RPoint.y = ip[1][i][ipsz2[1][i] - 1].y;
		p2.internal_circle[i].circle_edge.push_back(temp);
	}
	RedrawWindow();
	CClientDC cdc(this);
	for (int i = 0; i < p1.external_circle.circle_edge.size(); i++){
		CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
		CPen *pOldPen = cdc.SelectObject(&pen);
		CPoint t1, t2;
		t1.x = p1.external_circle.circle_edge[i].LPoint.x;
		t1.y = p1.external_circle.circle_edge[i].LPoint.y;
		t2.x = p1.external_circle.circle_edge[i].RPoint.x;
		t2.y = p1.external_circle.circle_edge[i].RPoint.y;
		cdc.MoveTo(t1);
		cdc.LineTo(t2);
	}
	for (int j = 0; j < p1.internal_circle.size(); j++){
		for (int i = 0; i < p1.internal_circle[j].circle_edge.size(); i++){
			CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
			CPen *pOldPen = cdc.SelectObject(&pen);
			CPoint t1, t2;
			t1.x = p1.internal_circle[j].circle_edge[i].LPoint.x;
			t1.y = p1.internal_circle[j].circle_edge[i].LPoint.y;
			t2.x = p1.internal_circle[j].circle_edge[i].RPoint.x;
			t2.y = p1.internal_circle[j].circle_edge[i].RPoint.y;
			cdc.MoveTo(t1);
			cdc.LineTo(t2);
		}
	}
	for (int i = 0; i < p2.external_circle.circle_edge.size(); i++){
		CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
		CPen *pOldPen = cdc.SelectObject(&pen);
		CPoint t1, t2;
		t1.x = p2.external_circle.circle_edge[i].LPoint.x;
		t1.y = p2.external_circle.circle_edge[i].LPoint.y;
		t2.x = p2.external_circle.circle_edge[i].RPoint.x;
		t2.y = p2.external_circle.circle_edge[i].RPoint.y;
		cdc.MoveTo(t1);
		cdc.LineTo(t2);
	}
	for (int j = 0; j < p2.internal_circle.size(); j++){
		for (int i = 0; i < p2.internal_circle[j].circle_edge.size(); i++){
			CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
			CPen *pOldPen = cdc.SelectObject(&pen);
			CPoint t1, t2;
			t1.x = p2.internal_circle[j].circle_edge[i].LPoint.x;
			t1.y = p2.internal_circle[j].circle_edge[i].LPoint.y;
			t2.x = p2.internal_circle[j].circle_edge[i].RPoint.x;
			t2.y = p2.internal_circle[j].circle_edge[i].RPoint.y;
			cdc.MoveTo(t1);
			cdc.LineTo(t2);
		}
	}
	solve_or(p1, p2, &cdc);
}


void CMFCApplication3View::OnAsubb()
{
	// TODO:  在此添加命令处理程序代码
	// TODO:  在此添加命令处理程序代码
	MPolygon p1, p2;
	Edge temp;
	for (int i = 0; i < epsz[0] - 1; i++){

		temp.LPoint.x = ep[0][i].x;
		temp.LPoint.y = ep[0][i].y;
		temp.RPoint.x = ep[0][i + 1].x;
		temp.RPoint.y = ep[0][i + 1].y;
		p1.external_circle.circle_edge.push_back(temp);
	}
	temp.LPoint.x = ep[0][0].x;
	temp.LPoint.y = ep[0][0].y;
	temp.RPoint.x = ep[0][epsz[0] - 1].x;
	temp.RPoint.y = ep[0][epsz[0] - 1].y;
	p1.external_circle.circle_edge.push_back(temp);
	for (int i = 0; i < ipsz1[0]; i++){
		Circle tc;
		p1.internal_circle.push_back(tc);
		for (int j = 0; j < ipsz2[0][i] - 1; j++){
			temp.LPoint.x = ip[0][i][j].x;
			temp.LPoint.y = ip[0][i][j].y;
			temp.RPoint.x = ip[0][i][j + 1].x;
			temp.RPoint.y = ip[0][i][j + 1].y;
			p1.internal_circle[i].circle_edge.push_back(temp);
		}
		temp.LPoint.x = ip[0][i][0].x;
		temp.LPoint.y = ip[0][i][0].y;
		temp.RPoint.x = ip[0][i][ipsz2[0][i] - 1].x;
		temp.RPoint.y = ip[0][i][ipsz2[0][i] - 1].y;
		p1.internal_circle[i].circle_edge.push_back(temp);
	}
	for (int i = 0; i < epsz[1] - 1; i++){
		temp.LPoint.x = ep[1][i].x;
		temp.LPoint.y = ep[1][i].y;
		temp.RPoint.x = ep[1][i + 1].x;
		temp.RPoint.y = ep[1][i + 1].y;
		p2.external_circle.circle_edge.push_back(temp);
	}
	temp.LPoint.x = ep[1][0].x;
	temp.LPoint.y = ep[1][0].y;
	temp.RPoint.x = ep[1][epsz[1] - 1].x;
	temp.RPoint.y = ep[1][epsz[1] - 1].y;
	p2.external_circle.circle_edge.push_back(temp);
	for (int i = 0; i < ipsz1[1]; i++){
		Circle tc;
		p2.internal_circle.push_back(tc);
		for (int j = 0; j < ipsz2[1][i] - 1; j++){
			temp.LPoint.x = ip[1][i][j].x;
			temp.LPoint.y = ip[1][i][j].y;
			temp.RPoint.x = ip[1][i][j + 1].x;
			temp.RPoint.y = ip[1][i][j + 1].y;
			p2.internal_circle[i].circle_edge.push_back(temp);
		}
		temp.LPoint.x = ip[1][i][0].x;
		temp.LPoint.y = ip[1][i][0].y;
		temp.RPoint.x = ip[1][i][ipsz2[1][i] - 1].x;
		temp.RPoint.y = ip[1][i][ipsz2[1][i] - 1].y;
		p2.internal_circle[i].circle_edge.push_back(temp);
	}
	RedrawWindow();
	CClientDC cdc(this);
	for (int i = 0; i < p1.external_circle.circle_edge.size(); i++){
		CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
		CPen *pOldPen = cdc.SelectObject(&pen);
		CPoint t1, t2;
		t1.x = p1.external_circle.circle_edge[i].LPoint.x;
		t1.y = p1.external_circle.circle_edge[i].LPoint.y;
		t2.x = p1.external_circle.circle_edge[i].RPoint.x;
		t2.y = p1.external_circle.circle_edge[i].RPoint.y;
		cdc.MoveTo(t1);
		cdc.LineTo(t2);
	}
	for (int j = 0; j < p1.internal_circle.size(); j++){
		for (int i = 0; i < p1.internal_circle[j].circle_edge.size(); i++){
			CPen pen(PS_SOLID, 1, RGB(255, 0, 0));
			CPen *pOldPen = cdc.SelectObject(&pen);
			CPoint t1, t2;
			t1.x = p1.internal_circle[j].circle_edge[i].LPoint.x;
			t1.y = p1.internal_circle[j].circle_edge[i].LPoint.y;
			t2.x = p1.internal_circle[j].circle_edge[i].RPoint.x;
			t2.y = p1.internal_circle[j].circle_edge[i].RPoint.y;
			cdc.MoveTo(t1);
			cdc.LineTo(t2);
		}
	}
	for (int i = 0; i < p2.external_circle.circle_edge.size(); i++){
		CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
		CPen *pOldPen = cdc.SelectObject(&pen);
		CPoint t1, t2;
		t1.x = p2.external_circle.circle_edge[i].LPoint.x;
		t1.y = p2.external_circle.circle_edge[i].LPoint.y;
		t2.x = p2.external_circle.circle_edge[i].RPoint.x;
		t2.y = p2.external_circle.circle_edge[i].RPoint.y;
		cdc.MoveTo(t1);
		cdc.LineTo(t2);
	}
	for (int j = 0; j < p2.internal_circle.size(); j++){
		for (int i = 0; i < p2.internal_circle[j].circle_edge.size(); i++){
			CPen pen(PS_SOLID, 1, RGB(0, 255, 0));
			CPen *pOldPen = cdc.SelectObject(&pen);
			CPoint t1, t2;
			t1.x = p2.internal_circle[j].circle_edge[i].LPoint.x;
			t1.y = p2.internal_circle[j].circle_edge[i].LPoint.y;
			t2.x = p2.internal_circle[j].circle_edge[i].RPoint.x;
			t2.y = p2.internal_circle[j].circle_edge[i].RPoint.y;
			cdc.MoveTo(t1);
			cdc.LineTo(t2);
		}
	}
	solve_sub(p1, p2, &cdc);
}
