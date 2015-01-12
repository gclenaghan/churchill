#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>
#include "point_search.h"

#include <iostream>


struct SearchContext
{
	SearchContext(const Point* points_begin, const Point* points_end);
	int32_t search(const Rect rect, const int32_t count, Point* out_points);
	std::vector<Point> points;
};

class Cluster
{
	Cluster(std::vector<Point> p, Point c);
	void set_rank();
	void set_r2();
	std::vector<Point> points;
	int32_t rank;
	Point center;
	float r2;
};

inline bool is_inside(const Rect &rect, const Point &point)
{
	if( rect.lx <= point.x && rect.hx >= point.x &&
		rect.ly <= point.y && rect.hy >= point.y )
		return true;
	return false;
}

inline bool sortpoint(const Point a, const Point b)
{
	return a.rank < b.rank;
}

inline float d2(const Point a, const Point b)
{ //Square of distance between two points
	return (a.x - b.x)*(a.x - b.x) + (a.y - b.y)*(a.y - b.y);
}

//SearchContext definitions-------------------------------------------------------------------------------------
SearchContext::SearchContext(const Point* points_begin, const Point* points_end) : points(points_begin, points_end)
{
	std::sort(points.begin(), points.end(), sortpoint);
}

int32_t SearchContext::search(const Rect rect, const int32_t count, Point* out_points)
{
	int32_t result_count = 0;
	for(std::vector<Point>::iterator it = points.begin(); result_count < count && it != points.end(); ++it)
	{
		if (is_inside(rect, *it))
		{
			*out_points = *it;
			result_count++;
			out_points++;
		}
	}
	std::cout << "Max rank: " <<  (out_points-1)->rank << ".\n";
	return result_count;
}
//cluster definitions-------------------------------------------------------------------------------------------
Cluster::Cluster(std::vector<Point> p, Point c) : points(p) , rank(0) , r2(0)
{
	center = c;
}

void Cluster::set_rank()
{
	int32_t i = 0;
	for(std::vector<Point>::iterator it = points.begin(); it != points.end(); ++it)
	{
		i += it->rank;
	}

	rank = i;
}

void Cluster::set_r2()
{
	float maxr2 = 0;
	for(std::vector<Point>::iterator it = points.begin(); it != points.end(); ++it)
	{
		float testr2 = d2(center, *it);
		if (testr2 > maxr2)
		{
			maxr2 = testr2;
		}
	}
}


extern "C" __declspec(dllexport) SearchContext* __stdcall create(const Point* points_begin, const Point* points_end)
{
	return new SearchContext(points_begin, points_end);
}

extern "C" __declspec(dllexport) SearchContext* __stdcall destroy(SearchContext* sc)
{
	delete sc;
	return 0;
}

extern "C" __declspec(dllexport) int32_t __stdcall search(SearchContext* sc, const Rect rect, const int32_t count, Point* out_points)
{
	return sc->search(rect, count, out_points);
}
