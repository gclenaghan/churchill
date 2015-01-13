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
	return result_count;
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
