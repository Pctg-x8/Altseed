﻿
#include "asd.Terrain3D_Imp.h"
#include "../../asd.Graphics_Imp.h"

#include "../../Resource/asd.VertexBuffer_Imp.h"
#include "../../Resource/asd.IndexBuffer_Imp.h"

#include "../../Resource/asd.Material3D.h"
#include "../../Resource/asd.Shader3D.h"
#include "../../Resource/asd.Shader3D_Imp.h"

#include <Utility/asd.BinaryWriter.h>

namespace asd
{
	enum class DivisionDirection
	{
		None_,
		Filled,
		FilledHalf,
		Slash_Upper,
		Slash_Lower,
		Backslash_Upper,
		Backslash_Lower,
	};

	static std::vector<std::pair<int32_t, int32_t>> ExtractLines(std::array<DivisionDirection, 16>& divisions)
	{
		std::vector<std::pair<int32_t, int32_t>> lines;

		for (int32_t i = 0; i < 16; i++)
		{
			auto x = i % 4;
			auto y = i / 4;

			auto d = divisions[i];
			if (d == DivisionDirection::None_) continue;
			if (d == DivisionDirection::Backslash_Lower)
			{
				lines.push_back(std::pair<int32_t, int32_t>((x + 1) + (y + 1) * 5, (x + 0) + (y + 0) * 5));
			}
			else if (d == DivisionDirection::Backslash_Upper)
			{
				lines.push_back(std::pair<int32_t, int32_t>((x + 0) + (y + 0) * 5, (x + 1) + (y + 1) * 5));
			}
			else if (d == DivisionDirection::Slash_Lower)
			{
				lines.push_back(std::pair<int32_t, int32_t>((x + 1) + (y + 0) * 5, (x + 0) + (y + 1) * 5));
			}
			else if (d == DivisionDirection::Slash_Upper)
			{
				lines.push_back(std::pair<int32_t, int32_t>((x + 0) + (y + 1) * 5, (x + 1) + (y + 0) * 5));
			}
			else if (d == DivisionDirection::Filled)
			{
				
				auto isNone = [&divisions](int32_t x_, int32_t y_) -> bool
				{
					if (x_ < 0) return false;
					if (x_ >= 4) return false;
					if (y_ < 0) return false;
					if (y_ >= 4) return false;
					return divisions[x_ + y_ * 4] == DivisionDirection::None_;
				};

				if (isNone(x - 1, y))
				{
					lines.push_back(std::pair<int32_t, int32_t>((x + 0) + (y + 0) * 5, (x + 0) + (y + 1) * 5));
				}

				if (isNone(x + 1, y))
				{
					lines.push_back(std::pair<int32_t, int32_t>((x + 1) + (y + 1) * 5, (x + 1) + (y + 0) * 5));
				}

				if (isNone(x, y - 1))
				{
					lines.push_back(std::pair<int32_t, int32_t>((x + 1) + (y + 0) * 5, (x + 0) + (y + 0) * 5));
				}

				if (isNone(x, y + 1))
				{
					lines.push_back(std::pair<int32_t, int32_t>((x + 0) + (y + 1) * 5, (x + 1) + (y + 1) * 5));
				}
				
			}
		}

		return lines;
	}

	static void CalculateDivisionState(std::array<DivisionDirection, 16>& dst, bool isCliffes[9], bool isInclinedPlanes[9])
	{
		auto rot = [](int32_t ind, int32_t angle) -> int32_t
		{
			int32_t lut[9];
			lut[0] = 2;
			lut[1] = 5;
			lut[2] = 8;
			lut[3] = 1;
			lut[4] = 4;
			lut[5] = 7;
			lut[6] = 0;
			lut[7] = 3;
			lut[8] = 6;

			for (int32_t i = 0; i < angle; i++)
			{
				ind = lut[ind];
			}
			return ind;
		};

		auto rot_16 = [](int32_t ind, int32_t angle) -> int32_t
		{
			auto x = ind % 4;
			auto y = ind / 4;

			for (int32_t i = 0; i < angle; i++)
			{
				auto x_ = 3 - y;
				auto y_ = x;
				x = x_;
				y = y_;
			}
			return x + y * 4;
		};

		auto rotDiv = [](DivisionDirection d, int32_t angle) -> DivisionDirection
		{
			if (d == DivisionDirection::Filled) return d;
			if (d == DivisionDirection::FilledHalf) return d;
			if (d == DivisionDirection::None_) return d;

			for (int32_t i = 0; i < angle; i++)
			{
				if (d == DivisionDirection::Backslash_Lower)
				{
					d = DivisionDirection::Slash_Upper;
				}
				else if (d == DivisionDirection::Backslash_Upper)
				{
					d = DivisionDirection::Slash_Lower;
				}
				else if (d == DivisionDirection::Slash_Lower)
				{
					d = DivisionDirection::Backslash_Lower;
				}
				else if (d == DivisionDirection::Slash_Upper)
				{
					d = DivisionDirection::Backslash_Upper;
				}
			}

			return d;
		};

		if (!isCliffes[0] && !isCliffes[1] && !isCliffes[2] &&
			!isCliffes[3] && !isCliffes[4] && !isCliffes[5] &&
			!isCliffes[6] && !isCliffes[7] && !isCliffes[8])
		{
			// なし
			for (int32_t i = 0; i < 16; i++)
			{
				dst[i] = DivisionDirection::None_;
			}
		}
		else if (isCliffes[1] && isCliffes[3] && isCliffes[6] && isCliffes[8])
		{
			// 十字方向のうち4方向が崖
			dst[0 + 4 * 0] = DivisionDirection::None_;
			dst[1 + 4 * 0] = DivisionDirection::None_;
			dst[2 + 4 * 0] = DivisionDirection::None_;
			dst[3 + 4 * 0] = DivisionDirection::None_;

			dst[0 + 4 * 1] = DivisionDirection::None_;
			dst[1 + 4 * 1] = DivisionDirection::Slash_Lower;
			dst[2 + 4 * 1] = DivisionDirection::Backslash_Lower;
			dst[3 + 4 * 1] = DivisionDirection::None_;

			dst[0 + 4 * 2] = DivisionDirection::None_;
			dst[1 + 4 * 2] = DivisionDirection::Backslash_Upper;
			dst[2 + 4 * 2] = DivisionDirection::Slash_Upper;
			dst[3 + 4 * 2] = DivisionDirection::None_;

			dst[0 + 4 * 3] = DivisionDirection::None_;
			dst[1 + 4 * 3] = DivisionDirection::None_;
			dst[2 + 4 * 3] = DivisionDirection::None_;
			dst[3 + 4 * 3] = DivisionDirection::None_;

			goto Exit;
		}
		else
		{
			for (int32_t i = 0; i < 16; i++)
			{
				dst[i] = DivisionDirection::Filled;
			}

			// 十字方向のうち3方向が崖
			for (int32_t dir = 0; dir < 4; dir++)
			{
				if (isCliffes[rot(3, dir)] && isCliffes[rot(1, dir)] && isCliffes[rot(5, dir)])
				{
					dst[rot_16(0 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(1 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(2 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(3 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);

					dst[rot_16(0 + 4 * 1, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(1 + 4 * 1, dir)] = rotDiv(DivisionDirection::Slash_Lower, dir);
					dst[rot_16(2 + 4 * 1, dir)] = rotDiv(DivisionDirection::Backslash_Upper, dir);
					dst[rot_16(3 + 4 * 1, dir)] = rotDiv(DivisionDirection::None_, dir);

					dst[rot_16(0 + 4 * 2, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(1 + 4 * 2, dir)] = rotDiv(DivisionDirection::Filled, dir);
					dst[rot_16(2 + 4 * 2, dir)] = rotDiv(DivisionDirection::Filled, dir);
					dst[rot_16(3 + 4 * 2, dir)] = rotDiv(DivisionDirection::None_, dir);

					dst[rot_16(0 + 4 * 3, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(1 + 4 * 3, dir)] = rotDiv(DivisionDirection::Filled, dir);
					dst[rot_16(2 + 4 * 3, dir)] = rotDiv(DivisionDirection::Filled, dir);
					dst[rot_16(3 + 4 * 3, dir)] = rotDiv(DivisionDirection::None_, dir);
					goto Exit;
				}
			}

			// 十字方向の2方向が崖でなく、斜め方向が崖
			for (int32_t dir = 0; dir < 4; dir++)
			{
				if (!isCliffes[rot(3, dir)] && isCliffes[rot(0, dir)] && !isCliffes[rot(1, dir)])
				{
					dst[rot_16(0 + 4 * 0, dir)] = rotDiv(DivisionDirection::Slash_Lower, dir);
				}
			}

			// 十字方向の2方向と斜め方向が崖
			for (int32_t dir = 0; dir < 4; dir++)
			{
				if (isCliffes[rot(3, dir)] &&isCliffes[rot(0, dir)] && isCliffes[rot(1, dir)])
				{
					dst[rot_16(0 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(1 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(2 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(3 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);

					dst[rot_16(0 + 4 * 1, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(1 + 4 * 1, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(2 + 4 * 1, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(3 + 4 * 1, dir)] = rotDiv(DivisionDirection::Slash_Lower, dir);

					dst[rot_16(0 + 4 * 2, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(1 + 4 * 2, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(2 + 4 * 2, dir)] = rotDiv(DivisionDirection::Slash_Lower, dir);

					dst[rot_16(0 + 4 * 3, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(1 + 4 * 3, dir)] = rotDiv(DivisionDirection::Slash_Lower, dir);
				}
			}

			for (int32_t dir = 0; dir < 4; dir++)
			{
				if (/*isCliffes[rot(0, dir)] &&*/ isCliffes[rot(1, dir)] && /*isCliffes[rot(2, dir)] &&*/ !isCliffes[rot(3, dir)] && !isCliffes[rot(5, dir)])
				{
					dst[rot_16(0 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(1 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(2 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);
					dst[rot_16(3 + 4 * 0, dir)] = rotDiv(DivisionDirection::None_, dir);

					if (isInclinedPlanes[rot(3, dir)] && !isInclinedPlanes[rot(5, dir)])
					{
					}
					else if (!isInclinedPlanes[rot(3, dir)] && isInclinedPlanes[rot(5, dir)])
					{
						dst[rot_16(0 + 4 * 1, dir)] = rotDiv(DivisionDirection::FilledHalf, dir);
						dst[rot_16(1 + 4 * 1, dir)] = rotDiv(DivisionDirection::FilledHalf, dir);
						//divisionsNext[2 + 4 * 1] = DivisionDirection::None_;
						//divisionsNext[3 + 4 * 1] = DivisionDirection::None_;
					}
					else if (isInclinedPlanes[rot(3, dir)] && !isInclinedPlanes[rot(5, dir)])
					{
						dst[rot_16(0 + 4 * 1, dir)] = rotDiv(DivisionDirection::None_, dir);
						dst[rot_16(1 + 4 * 1, dir)] = rotDiv(DivisionDirection::None_, dir);
						dst[rot_16(2 + 4 * 1, dir)] = rotDiv(DivisionDirection::FilledHalf, dir);
						dst[rot_16(3 + 4 * 1, dir)] = rotDiv(DivisionDirection::FilledHalf, dir);
					}
					else if (isInclinedPlanes[rot(3, dir)] && isInclinedPlanes[rot(5, dir)])
					{
						dst[rot_16(0 + 4 * 1, dir)] = rotDiv(DivisionDirection::FilledHalf, dir);
						dst[rot_16(1 + 4 * 1, dir)] = rotDiv(DivisionDirection::FilledHalf, dir);
						dst[rot_16(2 + 4 * 1, dir)] = rotDiv(DivisionDirection::FilledHalf, dir);
						dst[rot_16(3 + 4 * 1, dir)] = rotDiv(DivisionDirection::FilledHalf, dir);
					}
				}
			}
		}
	Exit:;
	}

	Terrain3D_Imp::Chip::Chip()
	{
		IsChanged = true;
		IsMeshGenerated = false;
		IsCollisionGenerated = false;
	}

	Terrain3D_Imp::Chip::~Chip()
	{
	}

	Terrain3D_Imp::CollisionCluster::CollisionCluster(Terrain3D_Imp* terrain, int32_t x, int32_t y, int32_t width, int32_t height)
	{
		CollisionMesh = nullptr;
		CollisionMeshShape = nullptr;
		CollisionObject = nullptr;

		this->terrain = terrain;
		this->x = x;
		this->y = y;
		this->width = width;
		this->height = height;
	}

	Terrain3D_Imp::CollisionCluster::~CollisionCluster()
	{
		if (CollisionObject != nullptr)
		{
			terrain->collisionWorld->removeCollisionObject(CollisionObject);
		}

		SafeDelete(CollisionMesh);
		SafeDelete(CollisionMeshShape);
		SafeDelete(CollisionObject);
	}

	void Terrain3D_Imp::CollisionCluster::GenerateCollision()
	{
		// 変更チェック
		bool isChanged = false;
		for (int32_t y_ = y; y_ < y + height; y_++)
		{
			for (int32_t x_ = x; x_ < x + width; x_++)
			{
				auto& chip = terrain->Chips[x_ + y_ * terrain->gridWidthCount];
				if (!chip.IsCollisionGenerated)
				{
					isChanged = true;
					break;
				}
			}
		}

		if (!isChanged) return;

		if (CollisionObject != nullptr)
		{
			terrain->collisionWorld->removeCollisionObject(CollisionObject);
		}
		
		SafeDelete(CollisionMesh);
		SafeDelete(CollisionMeshShape);
		SafeDelete(CollisionObject);

		CollisionMesh = new btTriangleMesh();

		for (int32_t y_ = y; y_ < y + height; y_++)
		{
			for (int32_t x_ = x; x_ < x + width; x_++)
			{
				auto& chip = terrain->Chips[x_ + y_ * terrain->gridWidthCount];
				chip.IsCollisionGenerated = true;

				for (size_t i = 0; i < chip.Faces.size(); i++)
				{
					const auto& pos0 = chip.Vertecies[chip.Faces[i].Indexes[0]];
					const auto& pos1 = chip.Vertecies[chip.Faces[i].Indexes[1]];
					const auto& pos2 = chip.Vertecies[chip.Faces[i].Indexes[2]];

					CollisionMesh->addTriangle(
						btVector3(pos0.X, pos0.Y, pos0.Z),
						btVector3(pos1.X, pos1.Y, pos1.Z),
						btVector3(pos2.X, pos2.Y, pos2.Z));
				}
			}
		}

		CollisionMeshShape = new btBvhTriangleMeshShape(CollisionMesh, true, true);
		CollisionObject = new btCollisionObject();
		CollisionObject->setCollisionShape(CollisionMeshShape);

		terrain->collisionWorld->addCollisionObject(CollisionObject, 1, 1);
	}

	Terrain3D_Imp::Terrain3D_Imp(Graphics* graphics)
		: m_graphics(graphics)
	{
		SafeAddRef(graphics);

		collisionConfiguration = new btDefaultCollisionConfiguration();
		collisionDispatcher = new btCollisionDispatcher(collisionConfiguration);
		btVector3 btv3WorldAabbMin(-10000.0f, -10000.0f, -10000.0f);
		btVector3 btv3WorldAabbMax(10000.0f, 10000.0f, 10000.0f);
		int32_t maxProxies = 1024;
		collisionOverlappingPairCache = new btAxisSweep3(btv3WorldAabbMin, btv3WorldAabbMax, maxProxies);

		collisionWorld = new btCollisionWorld(
			collisionDispatcher,
			collisionOverlappingPairCache,
			collisionConfiguration);
	}

	Terrain3D_Imp::~Terrain3D_Imp()
	{
		collisionClusters.clear();

		SafeDelete(collisionWorld);
		SafeDelete(collisionOverlappingPairCache);
		SafeDelete(collisionDispatcher);
		SafeDelete(collisionConfiguration);

		SafeRelease(m_graphics);
	}

	void Terrain3D_Imp::GenerateTerrainChip(int32_t chip_x, int32_t chip_y)
	{
		auto& chip = Chips[chip_x + chip_y * gridWidthCount];

		if (!chip.IsChanged) return;
		chip.IsChanged = false;

		chip.Vertecies.clear();
		chip.Faces.clear();

		int32_t clsh[9];

		for (int32_t oy = -1; oy <= 1; oy++)
		{
			for (int32_t ox = -1; ox <= 1; ox++)
			{
				auto x_ = Clamp(chip_x + ox, gridWidthCount - 1, 0);
				auto y_ = Clamp(chip_y + oy, gridHeightCount - 1, 0);
				clsh[(ox + 1) + (oy + 1) * 3] = cliffes[x_ + y_ * gridWidthCount];
			}
		}

		// 崖による4点の高度を決める
		int32_t clheight[4];
		
		for (int32_t oy = 0; oy < 2; oy++)
		{
			for (int32_t ox = 0; ox < 2; ox++)
			{
				int32_t m = INT_MAX;

				for (int32_t oy_ = 0; oy_ < 2; oy_++)
				{
					for (int32_t ox_ = 0; ox_ < 2; ox_++)
					{
						m = Min(m, clsh[(ox + ox_) + (oy + oy_) * 3]);
					}
				}

				clheight[ox + oy * 2] = m;
			}
		}

		auto getHeight = [this](int32_t x_, int32_t y_) -> float
		{
			x_ = Clamp(x_, this->gridWidthCount - 1, 0);
			y_ = Clamp(y_, this->gridHeightCount - 1, 0);

			return this->heights[x_ + y_ * this->gridWidthCount];
		};

		auto v00 = Vector3DF(
			chip_x * gridSize - gridWidthCount * gridSize / 2.0f,
			(getHeight(chip_x - 1, chip_y - 1) + getHeight(chip_x + 0, chip_y - 1) + getHeight(chip_x - 1, chip_y + 0) + getHeight(chip_x + 0, chip_y + 0)) / 4.0f,
			chip_y * gridSize - gridHeightCount * gridSize / 2.0f);

		auto v10 = Vector3DF(
			(chip_x + 1) * gridSize - gridWidthCount * gridSize / 2.0f,
			(getHeight(chip_x + 0, chip_y - 1) + getHeight(chip_x + 1, chip_y - 1) + getHeight(chip_x + 0, chip_y + 0) + getHeight(chip_x + 1, chip_y + 0)) / 4.0f,
			chip_y * gridSize - gridHeightCount * gridSize / 2.0f);

		auto v01 = Vector3DF(
			chip_x * gridSize - gridWidthCount * gridSize / 2.0f,
			(getHeight(chip_x - 1, chip_y + 0) + getHeight(chip_x + 0, chip_y + 0) + getHeight(chip_x - 1, chip_y + 1) + getHeight(chip_x + 0, chip_y + 1)) / 4.0f,
			(chip_y + 1) * gridSize - gridHeightCount * gridSize / 2.0f);

		auto v11 = Vector3DF(
			(chip_x + 1) * gridSize - gridWidthCount * gridSize / 2.0f,
			(getHeight(chip_x + 0, chip_y + 0) + getHeight(chip_x + 1, chip_y + 0) + getHeight(chip_x + 0, chip_y + 1) + getHeight(chip_x + 1, chip_y + 1)) / 4.0f,
			(chip_y + 1) * gridSize - gridHeightCount * gridSize / 2.0f);


		bool isFlat = true;

		for (int32_t i = 0; i < 9; i++)
		{
			if (clsh[4] - clsh[i] >= 2)
			{
				isFlat = false;
				break;
			}
		}

		if (isFlat)
		{
			// 平坦な場合
			v00.Y += clheight[0] * gridSize / 2.0f;
			v10.Y += clheight[1] * gridSize / 2.0f;
			v01.Y += clheight[2] * gridSize / 2.0f;
			v11.Y += clheight[3] * gridSize / 2.0f;

			chip.Vertecies.push_back(v00);
			chip.Vertecies.push_back(v10);
			chip.Vertecies.push_back(v01);
			chip.Vertecies.push_back(v11);

			ChipFace f1;
			f1.Indexes[0] = 0;
			f1.Indexes[1] = 1;
			f1.Indexes[2] = 3;

			ChipFace f2;
			f2.Indexes[0] = 0;
			f2.Indexes[1] = 3;
			f2.Indexes[2] = 2;

			chip.Faces.push_back(f1);
			chip.Faces.push_back(f2);
		}
		else
		{
			int32_t minclh = INT_MAX;
			int32_t maxclh = INT_MIN;
			for (int32_t oy = 0; oy < 3; oy++)
			{
				for (int32_t ox = 0; ox < 3; ox++)
				{
					minclh = Min(clsh[ox + oy * 3], minclh);
					maxclh = Max(clsh[ox + oy * 3], maxclh);
				}
			}

			maxclh = Min(maxclh, clsh[1 + 1 * 3]);

			std::array<DivisionDirection, 16> divisions;
			divisions.fill(DivisionDirection::None_);
			std::array<int32_t, 5*5> indexes;
			indexes.fill(-1);
			std::vector<std::pair<int32_t, int32_t>> lines;

			std::array<DivisionDirection, 16> divisionsNext;
			divisionsNext.fill(DivisionDirection::None_);
			std::array<int32_t, 5 * 5> indexesNext;
			indexesNext.fill(-1);
			std::vector<std::pair<int32_t, int32_t>> linesNext;

			for (int32_t h = maxclh; h >= minclh; h--)
			{
				// 面が存在する領域を抽出

				auto isCliff = [&clsh, h](int32_t i) -> bool
				{
					auto from = Min(clsh[4], h);
					auto to = Min(clsh[i], h);
					if (from <= to) return false;
					return from - to >= 2;
				};

				auto isInclinedPlane = [&clsh, h](int32_t i) -> bool
				{
					auto from = Min(clsh[4], h);
					auto to = Min(clsh[i], h);
					if (from <= to) return false;
					return from - to == 1;
				};
				
				bool isCliffes[9];
				bool isInclinedPlanes[9];
				for (int32_t i = 0; i < 9; i++)
				{
					isCliffes[i] = isCliff(i);
					isInclinedPlanes[i] = isInclinedPlane(i);
				}

				divisionsNext.fill(DivisionDirection::Filled);
				indexesNext.fill(-1);

				// このレイヤーに面があるか探索する
				bool isThisLayerExists = false;
				for (int32_t oy = 0; oy < 3; oy++)
				{
					for (int32_t ox = 0; ox < 3; ox++)
					{
						if (clsh[ox + oy * 3] == h)
						{
							isThisLayerExists = true;
							break;
						}
					}
				}


				// 領域ごとの状態を計算する。
				if (isThisLayerExists)
				{
					CalculateDivisionState(divisionsNext, isCliffes, isInclinedPlanes);
				}
				else
				{
					divisionsNext.fill(DivisionDirection::None_);
				}
				
				// 上の層の影響を下に与える。
				for (int32_t i = 0; i < 16; i++)
				{
					if (divisionsNext[i] == DivisionDirection::None_)
					{
						divisionsNext[i] = divisions[i];
					}
				}

				// 最下層強制補正
				if (h == minclh)
				{
					for (int32_t i = 0; i < 16; i++)
					{
						divisionsNext[i] = DivisionDirection::Filled;
					}
				}

				// 抽出した領域から面を形成する。

				// 輪郭を抽出する。
				linesNext = ExtractLines(divisionsNext);

				// 上の層と比較する
				// 広がった部分が面
				std::vector<Face> tempFaces;
				std::vector<std::pair<int32_t, int32_t>> squareFaces;
				std::vector<std::pair<int32_t, int32_t>> hSquareFaces;

				for (int32_t i = 0; i < 16; i++)
				{
					auto x = i % 4;
					auto y = i / 4;

					if (divisionsNext[i] == DivisionDirection::None_) continue;
					if (divisions[i] == divisionsNext[i]) continue;

					if (divisions[i] == DivisionDirection::None_ && divisionsNext[i] != DivisionDirection::None_)
					{
						// 面追加
						if (divisionsNext[i] == DivisionDirection::Backslash_Lower)
						{
							Face f;
							f.Index1 = (x + 0) + (y + 0) * 5;
							f.Index2 = (x + 1) + (y + 1) * 5;
							f.Index3 = (x + 0) + (y + 1) * 5;
							tempFaces.push_back(f);
						}

						if (divisionsNext[i] == DivisionDirection::Backslash_Upper)
						{
							Face f;
							f.Index1 = (x + 0) + (y + 0) * 5;
							f.Index2 = (x + 1) + (y + 0) * 5;
							f.Index3 = (x + 1) + (y + 1) * 5;
							tempFaces.push_back(f);
						}

						if (divisionsNext[i] == DivisionDirection::Slash_Lower)
						{
							Face f;
							f.Index1 = (x + 1) + (y + 0) * 5;
							f.Index2 = (x + 1) + (y + 1) * 5;
							f.Index3 = (x + 0) + (y + 1) * 5;
							tempFaces.push_back(f);
						}

						if (divisionsNext[i] == DivisionDirection::Slash_Upper)
						{
							Face f;
							f.Index1 = (x + 0) + (y + 0) * 5;
							f.Index2 = (x + 1) + (y + 0) * 5;
							f.Index3 = (x + 0) + (y + 1) * 5;
							tempFaces.push_back(f);
						}

						if (divisionsNext[i] == DivisionDirection::Filled)
						{
							squareFaces.push_back(std::pair<int32_t, int32_t>(x, y));
						}

						if (divisionsNext[i] == DivisionDirection::FilledHalf)
						{
							hSquareFaces.push_back(std::pair<int32_t, int32_t>(x, y));
						}
					}
					else if (divisions[i] == DivisionDirection::Slash_Lower && divisionsNext[i] == DivisionDirection::Filled)
					{
						Face f;
						f.Index1 = (x + 0) + (y + 0) * 5;
						f.Index2 = (x + 1) + (y + 0) * 5;
						f.Index3 = (x + 0) + (y + 1) * 5;
						tempFaces.push_back(f);
					}
					else if (divisions[i] == DivisionDirection::Slash_Upper && divisionsNext[i] == DivisionDirection::Filled)
					{
						Face f;
						f.Index1 = (x + 1) + (y + 0) * 5;
						f.Index2 = (x + 1) + (y + 1) * 5;
						f.Index3 = (x + 0) + (y + 1) * 5;
						tempFaces.push_back(f);
					}
					else if (divisions[i] == DivisionDirection::Backslash_Lower && divisionsNext[i] == DivisionDirection::Filled)
					{
						Face f;
						f.Index1 = (x + 0) + (y + 0) * 5;
						f.Index2 = (x + 1) + (y + 0) * 5;
						f.Index3 = (x + 1) + (y + 1) * 5;
						tempFaces.push_back(f);
					}
					else if (divisions[i] == DivisionDirection::Backslash_Upper && divisionsNext[i] == DivisionDirection::Filled)
					{
						Face f;
						f.Index1 = (x + 0) + (y + 0) * 5;
						f.Index2 = (x + 1) + (y + 1) * 5;
						f.Index3 = (x + 0) + (y + 1) * 5;
						tempFaces.push_back(f);
					}
				}

				// 面追加

				auto v00_ = v00;
				auto v10_ = v10;
				auto v01_ = v01;
				auto v11_ = v11;

				v00_.Y += h * gridSize / 2.0f;
				v10_.Y += h * gridSize / 2.0f;
				v01_.Y += h * gridSize / 2.0f;
				v11_.Y += h * gridSize / 2.0f;

				for (auto& f : tempFaces)
				{
					if (indexesNext[f.Index1] == -1)
					{
						auto x = f.Index1 % 5;
						auto y = f.Index1 / 5;

						auto v0 = (v01_ - v00_) * (y / 4.0f) + v00_;
						auto v1 = (v11_ - v10_) * (y / 4.0f) + v10_;
						auto v = (v1 - v0) * (x / 4.0f) + v0;
						chip.Vertecies.push_back(v);
						indexesNext[f.Index1] = chip.Vertecies.size() - 1;
					}

					if (indexesNext[f.Index2] == -1)
					{
						auto x = f.Index2 % 5;
						auto y = f.Index2 / 5;

						auto v0 = (v01_ - v00_) * (y / 4.0f) + v00_;
						auto v1 = (v11_ - v10_) * (y / 4.0f) + v10_;
						auto v = (v1 - v0) * (x / 4.0f) + v0;
						chip.Vertecies.push_back(v);
						indexesNext[f.Index2] = chip.Vertecies.size() - 1;
					}

					if (indexesNext[f.Index3] == -1)
					{
						auto x = f.Index3 % 5;
						auto y = f.Index3 / 5;

						auto v0 = (v01_ - v00_) * (y / 4.0f) + v00_;
						auto v1 = (v11_ - v10_) * (y / 4.0f) + v10_;
						auto v = (v1 - v0) * (x / 4.0f) + v0;
						chip.Vertecies.push_back(v);
						indexesNext[f.Index3] = chip.Vertecies.size() - 1;
					}
				}

				for (auto& sf : squareFaces)
				{
					auto x = sf.first;
					auto y = sf.second;

					int32_t indexes[4];
					indexes[0] = (x + 0) + (y + 0) * 5;
					indexes[1] = (x + 1) + (y + 0) * 5;
					indexes[2] = (x + 0) + (y + 1) * 5;
					indexes[3] = (x + 1) + (y + 1) * 5;

					for (int32_t i = 0; i < 4; i++)
					{
						if (indexesNext[indexes[i]] == -1)
						{
							auto x = indexes[i] % 5;
							auto y = indexes[i] / 5;

							auto v0 = (v01_ - v00_) * (y / 4.0f) + v00_;
							auto v1 = (v11_ - v10_) * (y / 4.0f) + v10_;
							auto v = (v1 - v0) * (x / 4.0f) + v0;
							chip.Vertecies.push_back(v);
							indexesNext[indexes[i]] = chip.Vertecies.size() - 1;
						}
					}

					ChipFace f1;
					f1.Indexes[0] = indexesNext[indexes[0]];
					f1.Indexes[1] = indexesNext[indexes[1]];
					f1.Indexes[2] = indexesNext[indexes[3]];

					ChipFace f2;
					f2.Indexes[0] = indexesNext[indexes[0]];
					f2.Indexes[1] = indexesNext[indexes[3]];
					f2.Indexes[2] = indexesNext[indexes[2]];

					chip.Faces.push_back(f1);
					chip.Faces.push_back(f2);
				}

				for (auto& f : tempFaces)
				{
					ChipFace f_;
					f_.Indexes[0] = indexesNext[f.Index1];
					f_.Indexes[1] = indexesNext[f.Index2];
					f_.Indexes[2] = indexesNext[f.Index3];

					chip.Faces.push_back(f_);
				}

				// 側面形成
				for (auto& line : lines)
				{
					if (indexesNext[line.first] == -1)
					{
						auto x = line.first % 5;
						auto y = line.first / 5;

						auto v0 = (v01_ - v00_) * (y / 4.0f) + v00_;
						auto v1 = (v11_ - v10_) * (y / 4.0f) + v10_;
						auto v = (v1 - v0) * (x / 4.0f) + v0;
						chip.Vertecies.push_back(v);
						indexesNext[line.first] = chip.Vertecies.size() - 1;
					}
					
					if (indexesNext[line.second] == -1)
					{
						auto x = line.second % 5;
						auto y = line.second / 5;

						auto v0 = (v01_ - v00_) * (y / 4.0f) + v00_;
						auto v1 = (v11_ - v10_) * (y / 4.0f) + v10_;
						auto v = (v1 - v0) * (x / 4.0f) + v0;
						chip.Vertecies.push_back(v);
						indexesNext[line.second] = chip.Vertecies.size() - 1;
					}
				}

				for (auto& line : lines)
				{
					if (indexes[line.first] == -1) continue;
					if (indexes[line.second] == -1) continue;
					if (indexesNext[line.first] == -1) continue;
					if (indexesNext[line.second] == -1) continue;

					ChipFace f1;
					f1.Indexes[0] = indexes[line.first];
					f1.Indexes[1] = indexes[line.second];
					f1.Indexes[2] = indexesNext[line.second];

					ChipFace f2;
					f2.Indexes[0] = indexes[line.first];
					f2.Indexes[1] = indexesNext[line.second];
					f2.Indexes[2] = indexesNext[line.first];

					chip.Faces.push_back(f1);
					chip.Faces.push_back(f2);
				}

				indexes = indexesNext;
				divisions = divisionsNext;
				lines = linesNext;
			}
		}

		// 面計算
		for (size_t i = 0; i < chip.Faces.size(); i++)
		{
			auto& face = chip.Faces[i];

			auto normal = Vector3DF::Cross(
				(chip.Vertecies[face.Indexes[2]] - chip.Vertecies[face.Indexes[0]]),
				(chip.Vertecies[face.Indexes[1]] - chip.Vertecies[face.Indexes[0]]));

			normal.Normalize();

			face.Normal = normal;

			if (abs(Vector3DF::Dot(normal, Vector3DF(0, 0, 1))) < 0.9f)
			{
				auto tangent = Vector3DF::Cross(normal, Vector3DF(0, 0, 1));
				tangent.Normalize();

				face.Binormal = Vector3DF::Cross(tangent, normal);
				face.Binormal.Normalize();
			}
			else
			{
				auto binormal = Vector3DF::Cross(Vector3DF(1, 0, 0), normal);
				binormal.Normalize();

				face.Binormal = binormal;
			}
			
		}
	}

	void Terrain3D_Imp::GenerateTerrainChips()
	{
		for (int32_t y = 0; y < gridHeightCount; y++)
		{
			for (int32_t x = 0; x < gridWidthCount; x++)
			{
				GenerateTerrainChip(x, y);
			}
		}
	}

	void Terrain3D_Imp::GenerateTerrainMesh(int32_t chip_x, int32_t chip_y, int32_t chip_width, int32_t chip_height, std::vector<Vertex>& vertices, std::vector<Face>& faces)
	{
		std::vector<Vector3DF> chipVertices;
		std::vector<ChipFace> chipFaces;
		std::map<Vector3DF, int32_t> chipVertexPositionToVertexIndexes;
		std::map<Vector3DF, std::vector<int32_t>> chipVertexPositionToFaceIndexes;

		auto cx_min = Clamp(chip_x - 1, gridWidthCount - 1, 0);
		auto cy_min = Clamp(chip_y - 1, gridHeightCount - 1, 0);
		auto cx_max = Clamp(chip_x + chip_width + 1, gridWidthCount - 1, 0);
		auto cy_max = Clamp(chip_y + chip_height + 1, gridHeightCount - 1, 0);

		for (size_t y = cy_min; y <= cy_max; y++)
		{
			for (size_t x = cx_min; x <= cx_max; x++)
			{
				auto indexOffset = chipVertices.size();

				auto& chip = Chips[x + y * gridWidthCount];

				for (auto& v : chip.Vertecies)
				{
					auto ind = chipVertexPositionToFaceIndexes.find(v);
					if (ind == chipVertexPositionToFaceIndexes.end())
					{
						chipVertices.push_back(v);
						chipVertexPositionToVertexIndexes[v] = (int32_t) (chipVertices.size() - 1);
						chipVertexPositionToFaceIndexes[v] = std::vector<int32_t>();
					}
				}

				for (auto& f : chip.Faces)
				{
					auto f_ = f;
					for (size_t i = 0; i < 3; i++)
					{
						auto v = chip.Vertecies[f.Indexes[i]];
						f_.Indexes[i] = chipVertexPositionToVertexIndexes[v];
						chipVertexPositionToFaceIndexes[v].push_back(chipFaces.size());
					}

					chipFaces.push_back(f_);
				}
			}
		}

		vertices.clear();
		faces.clear();

		for (size_t i = 0; i < chipVertices.size(); i++)
		{
			auto v = chipVertices[i];
			Vertex cv;

			cv.Position = v;

			Vector3DF normal;
			Vector3DF binormal;
			for (auto ind : chipVertexPositionToFaceIndexes[v])
			{
				auto f = chipFaces[ind];
				normal += f.Normal;
				binormal += f.Binormal;
			}

			normal /= (float) chipVertexPositionToFaceIndexes[v].size();
			binormal /= (float) chipVertexPositionToFaceIndexes[v].size();

			cv.Normal = normal;
			cv.Binormal = binormal;

			vertices.push_back(cv);
		}

		for (size_t i = 0; i < chipFaces.size(); i++)
		{
			auto f = chipFaces[i];
			Face face;
			face.Index1 = f.Indexes[0];
			face.Index2 = f.Indexes[1];
			face.Index3 = f.Indexes[2];
			faces.push_back(face);
		}

		std::map<int32_t, int32_t> indToNewInd;

		auto tempV = vertices;
		vertices.clear();
		
		for (size_t i = 0; i < tempV.size(); i++)
		{
			auto v = tempV[i];

			if (v.Position.X < chip_x * gridSize - gridWidthCount * gridSize / 2.0f ||
				v.Position.X >(chip_x + chip_width) * gridSize - gridWidthCount * gridSize / 2.0f ||
				v.Position.Z < chip_y * gridSize - gridHeightCount * gridSize / 2.0f ||
				v.Position.Z >(chip_y + chip_height) * gridSize - gridHeightCount * gridSize / 2.0f)
			{
			}
			else
			{
				indToNewInd[i] = vertices.size();
				vertices.push_back(v);
			}
		}

		auto tempF = faces;
		faces.clear();

		for (size_t i = 0; i < tempF.size(); i++)
		{
			auto f = tempF[i];

			if (indToNewInd.count(f.Index1) > 0)
			{
				f.Index1 = indToNewInd[f.Index1];
			}
			else
			{
				continue;
			}

			if (indToNewInd.count(f.Index2) > 0)
			{
				f.Index2 = indToNewInd[f.Index2];
			}
			else
			{
				continue;
			}

			if (indToNewInd.count(f.Index3) > 0)
			{
				f.Index3 = indToNewInd[f.Index3];
			}
			else
			{
				continue;
			}

			faces.push_back(f);
		}
	}

	bool Terrain3D_Imp::Commit()
	{
		if (!isSurfaceChanged && !isMeshChanged) return false;
		
		isMeshChanged = false;

		GenerateTerrainChips();

		for (auto& c : collisionClusters)
		{
			c->GenerateCollision();
		}

		collisionWorld->updateAabbs();

		auto g = (Graphics_Imp*) m_graphics;

		Proxy.GridWidthCount = gridWidthCount;
		Proxy.GridHeightCount = gridHeightCount;
		Proxy.GridSize = gridSize;
		Proxy.Material_ = material_;

		if (isSurfaceChanged)
		{
			Proxy.Surfaces.clear();
			for (auto& surface : surfaceNameToSurface)
			{
				SurfaceProxy p;

				p.ColorTexture = surface.second.ColorTexture;
				p.NormalTexture = surface.second.NormalTexture;
				p.MetalnessTexture = surface.second.MetalnessTexture;

				auto ind = surfaceNameToIndex[surface.first];
				auto& surface_ = surfaces[ind];

				p.DensityTexture = g->CreateEmptyTexture2D(
					gridWidthCount * pixelInGrid,
					gridHeightCount * pixelInGrid,
					TextureFormat::R8_UNORM);

				TextureLockInfomation info;
				if (p.DensityTexture->Lock(&info))
				{
					memcpy(info.GetPixels(), surface_.data(), (gridWidthCount * pixelInGrid) * (gridHeightCount * pixelInGrid));
					p.DensityTexture->Unlock();
				}

				Proxy.Surfaces.push_back(p);
			}

			isSurfaceChanged = false;
		}
		

		if (Proxy.GridWidthCount == 0) return true;
		if (Proxy.GridHeightCount == 0) return true;

		Proxy.ClusterWidthCount = Proxy.GridWidthCount / ClusterCount;
		Proxy.ClusterHeightCount = Proxy.GridHeightCount / ClusterCount;
		if (Proxy.ClusterWidthCount * ClusterCount != Proxy.GridWidthCount) Proxy.ClusterWidthCount++;
		if (Proxy.ClusterHeightCount * ClusterCount != Proxy.GridHeightCount) Proxy.ClusterHeightCount++;

		if (Proxy.ClusterWidthCount*Proxy.ClusterHeightCount != Proxy.Clusters.size())
		{
			Proxy.Clusters.resize(Proxy.ClusterWidthCount*Proxy.ClusterHeightCount);
		}
		
		for (auto cy = 0; cy < Proxy.ClusterHeightCount; cy++)
		{
			for (auto cx = 0; cx < Proxy.ClusterWidthCount; cx++)
			{
				auto xoffset = cx * ClusterCount;
				auto yoffset = cy * ClusterCount;
				auto width = Min(ClusterCount, Proxy.GridWidthCount - xoffset);
				auto height = Min(ClusterCount, Proxy.GridHeightCount - yoffset);

				bool isChanged = false;
				for (int32_t y = Max(yoffset - 1, 0); y < Min(yoffset + height + 1, gridHeightCount); y++)
				{
					for (int32_t x = Max(xoffset - 1, 0); x < Min(xoffset + width + 1, gridWidthCount); x++)
					{
						auto& chip = Chips[x + gridWidthCount * y];
						if (!chip.IsMeshGenerated)
						{
							isChanged = true;
						}
						chip.IsMeshGenerated = true;
					}
				}

				if (!isChanged) continue;

				Proxy.Clusters[cx + cy * Proxy.ClusterWidthCount] = std::make_shared<ClusterProxy>();
				auto& cluster = Proxy.Clusters[cx + cy * Proxy.ClusterWidthCount];

				std::vector<Vertex> vs;
				std::vector<Face> fs;
				GenerateTerrainMesh(xoffset, yoffset, width, height, vs, fs);

				cluster->Size.X = width * gridSize;
				cluster->Size.Z = height * gridSize;

				cluster->Center.X = (xoffset + width / 2) * gridSize - gridWidthCount * gridSize / 2.0f;
				cluster->Center.Z = (yoffset + height / 2) * gridSize - gridHeightCount * gridSize / 2.0f;

				// 下地
				cluster->Black.VB = g->CreateVertexBuffer_Imp(sizeof(Vertex), vs.size(), false);
				cluster->Black.IB = g->CreateIndexBuffer_Imp(fs.size() * 3, false, true);

				{
					cluster->Black.VB->Lock();
					auto buf = cluster->Black.VB->GetBuffer<Vertex>(vs.size());
					for (auto i = 0; i < vs.size(); i++)
					{
						Vertex v = vs[i];
						v.VColor = Color(0, 0, 0, 255);
						buf[i] = v;
					}

					cluster->Black.VB->Unlock();
				}

				{
					cluster->Black.IB->Lock();
					auto buf = cluster->Black.IB->GetBuffer<int32_t>(fs.size() * 3);
					for (auto i = 0; i < fs.size(); i++)
					{
						buf[i * 3 + 0] = fs[i].Index1;
						buf[i * 3 + 1] = fs[i].Index2;
						buf[i * 3 + 2] = fs[i].Index3;
					}
					cluster->Black.IB->Unlock();
				}

				// サーフェイス
				int32_t surfaceInd = 0;
				for (auto& surface : surfaceNameToSurface)
				{
					auto ind_ = surfaceNameToIndex[surface.first];
					auto& surface_ = surfaces[ind_];

					// 内容確認
					bool hasPixel = false;
					for (auto y_ = yoffset * pixelInGrid; y_ < yoffset * pixelInGrid + height * pixelInGrid; y_++)
					{
						for (auto x_ = xoffset * pixelInGrid; x_ < xoffset * pixelInGrid + width * pixelInGrid; x_++)
						{
							if (surface_[x_ + y_ * gridWidthCount * pixelInGrid] > 0)
							{
								hasPixel = true;
								goto Exit;
							}
						}
					}
				Exit:;

					if (hasPixel)
					{
						SurfacePolygon p;
						p.SurfaceIndex = surfaceInd;

						p.VB = g->CreateVertexBuffer_Imp(sizeof(Vertex), vs.size(), false);
						p.IB = g->CreateIndexBuffer_Imp(fs.size() * 3, false, true);

						{
							p.VB->Lock();
							auto buf = p.VB->GetBuffer<Vertex>(vs.size());
							for (auto i = 0; i < vs.size(); i++)
							{
								Vertex v = vs[i];

								v.UV1.X = (v.Position.X + (gridWidthCount * gridSize / 2.0f)) / surface.second.Size;
								v.UV1.Y = (v.Position.Z + (gridHeightCount * gridSize / 2.0f)) / surface.second.Size;

								v.UV2.X = (v.Position.X + (gridWidthCount * gridSize / 2.0f)) / (float) gridSize / (float) (gridWidthCount);
								v.UV2.Y = (v.Position.Z + (gridHeightCount * gridSize / 2.0f)) / (float) gridSize / (float) (gridHeightCount);
							
								v.VColor = Color(255, 255, 255, 255);

								buf[i] = v;
							}

							p.VB->Unlock();
						}

						{
							p.IB->Lock();
							auto buf = p.IB->GetBuffer<int32_t>(fs.size() * 3);
							for (auto i = 0; i < fs.size(); i++)
							{
								buf[i * 3 + 0] = fs[i].Index1;
								buf[i * 3 + 1] = fs[i].Index2;
								buf[i * 3 + 2] = fs[i].Index3;
							}
							p.IB->Unlock();
						}

						cluster->Surfaces.push_back(p);
					}

					surfaceInd++;
				}
			}
		}

		return true;
	}

	void Terrain3D_Imp::New(float gridSize, int32_t gridWidthCount, int32_t gridHeightCount)
	{
		this->gridWidthCount = gridWidthCount;
		this->gridHeightCount = gridHeightCount;
		this->gridSize = gridSize;

		this->surfaceNameToIndex.clear();
		this->surfaceNameToSurface.clear();
		this->surfaces.clear();

		this->heights.resize(gridWidthCount * gridHeightCount);
		this->cliffes.resize(gridWidthCount * gridHeightCount);
		this->Chips.resize(gridWidthCount * gridHeightCount);

		for (size_t i = 0; i < this->heights.size(); i++)
		{
			this->heights[i] = 0.0f;
			this->cliffes[i] = 0;
		}

		GenerateTerrainChips();

		collisionClusters.clear();
		for (size_t y = 0; y < gridHeightCount; y += ClusterCount)
		{
			for (size_t x = 0; x < gridWidthCount; x += ClusterCount)
			{
				collisionClusters.push_back(
					std::make_shared<CollisionCluster>(
					this,
					x,
					y,
					Min(gridWidthCount - x, ClusterCount),
					Min(gridHeightCount - y, ClusterCount)
					));
			}
		}

		for (auto& c : collisionClusters)
		{
			c->GenerateCollision();
		}

		collisionWorld->updateAabbs();
		
		isMeshChanged = true;
		isSurfaceChanged = true;
	}

	void Terrain3D_Imp::LoadFromMemory(const std::vector<uint8_t>& buffer)
	{
		BinaryReader br;

		// TODO 高速化
		std::vector<uint8_t> temp;

		temp.resize(buffer.size());
		memcpy(temp.data(), buffer.data(), buffer.size());

		br.ReadIn(temp.begin(), temp.end());

		char* sig = "ater";
		uint8_t* sig_ = (uint8_t*) sig;

		for (int32_t i = 0; i < 4; i++)
		{
			auto s = br.Get<uint8_t>();
			if (sig_[i] != s) return;
		}

		// リセット
		this->surfaceNameToIndex.clear();
		this->surfaceNameToSurface.clear();
		this->surfaces.clear();

		// バージョン
		int32_t version = 0;
		version = br.Get<int32_t>();

		// グリッド
		this->gridWidthCount = br.Get<int32_t>();
		this->gridHeightCount = br.Get<int32_t>();
		this->gridSize = br.Get<float>();

		this->heights.resize(gridWidthCount * gridHeightCount);
		this->cliffes.resize(gridWidthCount * gridHeightCount);
		this->Chips.resize(gridWidthCount * gridHeightCount);

		for (size_t i = 0; i < this->heights.size(); i++)
		{
			this->cliffes[i] = 0;
		}

		// 地形
		for (size_t i = 0; i < this->heights.size(); i++)
		{
			this->heights[i] = br.Get<float>();
		}

		// サーフェス
		int32_t surfaceCount = br.Get<int32_t>();

		for (size_t i = 0; i < surfaceCount; i++)
		{
			auto name = br.Get<astring>();

			std::vector<uint8_t> surface;
			surface.resize((gridWidthCount * pixelInGrid) * (gridHeightCount * pixelInGrid));
			for (size_t p = 0; p < surface.size(); p++)
			{
				surface[p] = br.Get<uint8_t>();
			}
		}

		for (auto& c : Chips)
		{
			c.IsChanged = true;
		}

		GenerateTerrainChips();

		collisionClusters.clear();
		for (size_t y = 0; y < gridHeightCount; y += ClusterCount)
		{
			for (size_t x = 0; x < gridWidthCount; x += ClusterCount)
			{
				collisionClusters.push_back(
					std::make_shared<CollisionCluster>(
					this,
					x,
					y,
					Min(gridWidthCount - x, ClusterCount),
					Min(gridHeightCount - y, ClusterCount)
					));
			}
		}

		for (auto& c : collisionClusters)
		{
			c->GenerateCollision();
		}

		collisionWorld->updateAabbs();

		isMeshChanged = true;
		isSurfaceChanged = true;
	}

	std::vector<uint8_t> Terrain3D_Imp::SaveToMemory()
	{
		BinaryWriter bw;

		char* sig = "ater";
		uint8_t* sig_ = (uint8_t*) sig;

		for (int32_t i = 0; i < 4; i++)
		{
			bw.Push(sig_[i]);
		}

		// バージョン
		int32_t version = 0;
		bw.Push(version);

		// グリッド
		bw.Push(gridWidthCount);
		bw.Push(gridHeightCount);
		bw.Push(gridSize);

		// 地形
		for (size_t i = 0; i < this->heights.size(); i++)
		{
			bw.Push(this->heights[i]);
		}

		// サーフェース
		bw.Push((int32_t) surfaces.size());

		for (size_t i = 0; i < surfaces.size(); i++)
		{
			astring name;
			for (auto& kv : surfaceNameToIndex)
			{
				if (kv.second == i)
				{
					name = kv.first;
					break;
				}
			}

			auto& sf = surfaceNameToSurface[name];
			auto& surface = surfaces[i];

			bw.Push(name);

			for (size_t p = 0; p < surface.size(); p++)
			{
				bw.Push(surface[p]);
			}
		}

		// TODO 高速化
		std::vector<uint8_t> data;
		data.resize(bw.Get().size());
		memcpy(data.data(), bw.Get().data(), data.size());
		return data;
	}

	void Terrain3D_Imp::AddSurface(const achar* name, float size, const achar* color, const achar* normal, const achar* metalness)
	{
		Surface sf;
		sf.Size = size;

		sf.ColorPath = color;
		sf.NormalPath = normal;
		sf.MetalnessPath = metalness;

		sf.ColorTexture = m_graphics->CreateTexture2D(color);
		sf.NormalTexture = m_graphics->CreateTexture2DAsRawData(normal);
		sf.MetalnessTexture = m_graphics->CreateTexture2D(metalness);

		auto it = surfaceNameToIndex.find(name);

		if (it != surfaceNameToIndex.end())
		{
			surfaceNameToSurface[name] = sf;
		}
		else
		{
			surfaceNameToSurface[name] = sf;
			surfaceNameToIndex[name] = surfaces.size();
			surfaces.push_back(std::vector<uint8_t>());
			surfaces[surfaces.size() - 1].resize((gridWidthCount * pixelInGrid) * (gridHeightCount * pixelInGrid));

			if (surfaces.size() == 1)
			{
				for (auto i = 0; i < (gridWidthCount * pixelInGrid) * (gridHeightCount * pixelInGrid); i++)
				{
					surfaces[surfaces.size() - 1][i] = 255;
				}
			}
			else
			{
				for (auto i = 0; i < (gridWidthCount * pixelInGrid) * (gridHeightCount * pixelInGrid); i++)
				{
					surfaces[surfaces.size() - 1][i] = 0;
				}
			}
		}
	}

	int32_t Terrain3D_Imp::GetSurfaceIndex(const achar* name)
	{
		auto it = surfaceNameToIndex.find(name);

		if (it != surfaceNameToIndex.end())
		{
			return it->second;
		}

		return -1;
	}

	void Terrain3D_Imp::AssignSurfaceWithCircle(int32_t surfaceIndex, float x, float y, float radius, float value, float fallout)
	{
		if (surfaceIndex < 0) return;
		if (surfaceIndex >= surfaces.size()) return;

		if (fallout > 1.0f) fallout = 1.0f;
		if (fallout < 0.0f) fallout = 0.0f;

		x += gridWidthCount * gridSize / 2;
		y += gridHeightCount * gridSize / 2;

		x /= gridSize;
		y /= gridSize;
		radius /= gridSize;

		x *= pixelInGrid;
		y *= pixelInGrid;
		radius *= pixelInGrid;

		for (float y_ = y - radius; y_ < y + radius; y_ += 1.0f)
		{
			for (float x_ = x - radius; x_ < x + radius; x_ += 1.0f)
			{
				int32_t x_ind = (int32_t) x_;
				int32_t y_ind = (int32_t) y_;
				int32_t ind = x_ind + y_ind * (gridWidthCount * pixelInGrid);

				if (x_ind < 0) continue;
				if (x_ind >= gridWidthCount * pixelInGrid) continue;
				if (y_ind < 0) continue;
				if (y_ind >= gridHeightCount * pixelInGrid) continue;

				// ブラシの値を計算
				auto distance = sqrt((x_ - x) * (x_ - x) + (y_ - y) * (y_ - y));

				if (distance > radius) continue;

				auto variation = 0.0f;

				if (distance < radius * (1.0f-fallout))
				{
					variation = value;
				}
				else
				{
					variation = value * (1.0f - (distance - radius * (1.0f - fallout)) / (radius * fallout));
					
				}
					
				// 変化させる対象以外の合計値を計算
				int32_t sum = 0;
				for (int32_t i = 0; i < surfaces.size(); i++)
				{
					if (i != surfaceIndex) sum += surfaces[i][ind];
				}

				// 値を変化
				auto old_v = (int32_t)surfaces[surfaceIndex][ind];
				auto new_v = old_v + variation;

				if (new_v > 255) new_v = 255;
				if (new_v < 0) new_v = 0;

				auto diff = new_v - old_v;

				auto pert = (sum - diff) / (float) sum;

				// 変化した分他の値を修正
				for (int32_t i = 0; i < surfaces.size(); i++)
				{
					if (i != surfaceIndex) surfaces[i][ind] = surfaces[i][ind] * pert;
				}

				// 他の値の修正から値の変化を適用
				sum = 0;
				for (int32_t i = 0; i < surfaces.size(); i++)
				{
					if (i != surfaceIndex) sum += surfaces[i][ind];
				}

				if (sum > 255) sum = 255;
				if (sum < 0) sum = 0;

				surfaces[surfaceIndex][ind] = 255 - sum;

				// 塗られたグリッドのポリゴンを再生成
				{
					int32_t cind = x_ind / pixelInGrid + y_ind / pixelInGrid * (gridWidthCount);
					Chips[cind].IsMeshGenerated = false;
				}
			}
		}

		isSurfaceChanged = true;
	}

	void Terrain3D_Imp::SetMaterial(Material3D* material)
	{
		SafeAddRef(material);
		auto t = CreateSharedPtrWithReleaseDLL(material);
		material_ = t;

		if (material_ != nullptr)
		{
			auto shader = (Shader3D_Imp*) (material_->GetShader3D().get());
			shader->CompileTerrain();
		}

		isSurfaceChanged = true;
	}

	void Terrain3D_Imp::RaiseWithCircle(float x, float y, float radius, float value, float fallout)
	{
		if (fallout > 1.0f) fallout = 1.0f;
		if (fallout < 0.0f) fallout = 0.0f;

		x += gridWidthCount * gridSize / 2;
		y += gridHeightCount * gridSize / 2;

		x /= gridSize;
		y /= gridSize;
		radius /= gridSize;

		for (float y_ = y - radius; y_ < y + radius; y_ += 1.0f)
		{
			for (float x_ = x - radius; x_ < x + radius; x_ += 1.0f)
			{
				int32_t x_ind = (int32_t) x_;
				int32_t y_ind = (int32_t) y_;
				int32_t ind = x_ind + y_ind * (gridWidthCount);

				if (x_ind < 0) continue;
				if (x_ind >= gridWidthCount) continue;
				if (y_ind < 0) continue;
				if (y_ind >= gridHeightCount) continue;

				// 周囲に変更を通知
				for (int32_t cy = Max(y_ind - 1, 0); cy < Min(y_ind + 2, gridHeightCount); cy++)
				{
					for (int32_t cx = Max(x_ind - 1, 0); cx < Min(x_ind + 2, gridWidthCount); cx++)
					{
						int32_t cind = cx + cy * (gridWidthCount);

						Chips[cind].IsChanged = true;
						Chips[cind].IsMeshGenerated = false;
						Chips[cind].IsCollisionGenerated = false;
					}
				}

				// ブラシの値を計算
				auto distance = sqrt((x_ - x) * (x_ - x) + (y_ - y) * (y_ - y));

				if (distance > radius) continue;

				auto variation = 0.0f;

				if (distance < radius * (1.0f - fallout))
				{
					variation = value;
				}
				else
				{
					variation = value * (1.0f - (distance - radius * (1.0f - fallout)) / (radius * fallout));

				}

				heights[ind] += variation;
			}
		}

		isMeshChanged = true;
	}

	void Terrain3D_Imp::ChangeCliffesWithCircle(float x, float y, float radius, int32_t value)
	{
		x += gridWidthCount * gridSize / 2;
		y += gridHeightCount * gridSize / 2;

		x /= gridSize;
		y /= gridSize;
		radius /= gridSize;

		for (float y_ = y - radius; y_ < y + radius; y_ += 1.0f)
		{
			for (float x_ = x - radius; x_ < x + radius; x_ += 1.0f)
			{
				int32_t x_ind = (int32_t) x_;
				int32_t y_ind = (int32_t) y_;
				int32_t ind = x_ind + y_ind * (gridWidthCount);

				if (x_ind < 0) continue;
				if (x_ind >= gridWidthCount) continue;
				if (y_ind < 0) continue;
				if (y_ind >= gridHeightCount) continue;

				// 周囲に変更を通知
				for (int32_t cy = Max(y_ind - 1, 0); cy < Min(y_ind + 2, gridHeightCount); cy++)
				{
					for (int32_t cx = Max(x_ind - 1, 0); cx < Min(x_ind + 2, gridWidthCount); cx++)
					{
						int32_t cind = cx + cy * (gridWidthCount);

						Chips[cind].IsChanged = true;
						Chips[cind].IsMeshGenerated = false;
						Chips[cind].IsCollisionGenerated = false;
					}
				}

				// ブラシの値を計算
				auto distance = sqrt((x_ - x) * (x_ - x) + (y_ - y) * (y_ - y));

				if (distance > radius) continue;

				auto variation = 0.0f;

				if (distance < radius)
				{
					cliffes[ind] = value;
				}
			}
		}

		isMeshChanged = true;
	}

	Vector3DF Terrain3D_Imp::CastRay(const Vector3DF& from, const Vector3DF& to)
	{
		auto ret = Vector3DF();
		
		btVector3 from_(from.X, from.Y, from.Z);
		btVector3 to_(to.X, to.Y, to.Z);

		btCollisionWorld::ClosestRayResultCallback resultCallback(from_, to_);
		resultCallback.m_collisionFilterGroup = 1;
		resultCallback.m_collisionFilterMask = 1;

		collisionWorld->rayTest(from_, to_, resultCallback);
		if (resultCallback.hasHit())
		{
			ret.X = resultCallback.m_hitPointWorld.getX();
			ret.Y = resultCallback.m_hitPointWorld.getY();
			ret.Z = resultCallback.m_hitPointWorld.getZ();
		}
		else
		{
			ret.X = std::numeric_limits<float>::quiet_NaN();
			ret.Y = std::numeric_limits<float>::quiet_NaN();
			ret.Z = std::numeric_limits<float>::quiet_NaN();
		}

		return ret;
	}
}