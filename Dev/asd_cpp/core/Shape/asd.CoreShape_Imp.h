﻿#pragma once

#include "asd.ShapeType.h"
#include "../asd.Core.Base.h"
#include <Culling2D.h>

class b2Shape;

namespace asd{

	class CoreTriangleShape;

	class CoreShape_Imp
	{
	protected:
		bool GetIsCollidedb2Shapes(CoreShape* shape);
		bool GetIsCollidedWithCircleAndRect(CoreCircleShape* circle, CoreRectangleShape* rectangle);
		bool GetIsCollidedWithCircleAndLine(CoreCircleShape* circle, CoreLineShape* line);

	protected:
		std::vector<CoreTriangleShape*> triangles;
		std::vector<b2Shape*> collisionShapes;

		virtual void DivideToTriangles() = 0;

		virtual void CalcCollisions() = 0;

		culling2d::Circle boundingCircle;

		virtual void CalculateBoundingCircle() = 0;
	protected:
		bool isNeededUpdating;
		bool isNeededCalcBoundingCircle;
		bool isNeededCalcCollisions;

		virtual ShapeType GetType() const = 0;

	public:

		CoreShape_Imp() :
			isNeededUpdating(false)
			, isNeededCalcBoundingCircle(false)
			, isNeededCalcCollisions(false)
		{}
		virtual ~CoreShape_Imp();
#if !SWIG
		std::vector<CoreTriangleShape*>& GetDividedTriangles()
		{
			if (isNeededUpdating)
			{
				for (auto triangle : triangles)
				{
					SafeRelease(triangle);
				}
				triangles.clear();
				DivideToTriangles();
				isNeededUpdating = false;
			}
			return triangles;
		}

		culling2d::Circle& GetBoundingCircle()
		{
			if (isNeededCalcBoundingCircle)
			{
				CalculateBoundingCircle();
				isNeededCalcBoundingCircle = false;
			}
			return boundingCircle;
		}

		std::vector<b2Shape*>& GetCollisionShapes();
#endif
	};
};
