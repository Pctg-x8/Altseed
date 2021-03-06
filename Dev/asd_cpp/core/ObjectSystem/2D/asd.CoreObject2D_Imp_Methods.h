﻿int AddRef() { return ReferenceObject::AddRef(); }
int Release() { return ReferenceObject::Release(); }
int GetRef() { return ReferenceObject::GetRef(); }
virtual bool GetIsDrawn() const override { return CoreObject2D_Imp::GetIsDrawn(); }
virtual void SetIsDrawn(bool value) override { return CoreObject2D_Imp::SetIsDrawn(value); }
virtual bool GetIsAlive() const override { return CoreObject2D_Imp::GetIsAlive(); }
virtual void SetIsAlive(bool value) override { return CoreObject2D_Imp::SetIsAlive(value); }
virtual void SetLayer(CoreLayer2D* layer) override { return CoreObject2D_Imp::SetLayer(layer); }
virtual CoreLayer2D* GetLayer() override { return CoreObject2D_Imp::GetLayer(); }


virtual Vector2DF GetPosition() const override { return CoreObject2D_Imp::GetPosition(); }
virtual void SetPosition(Vector2DF value) override { return CoreObject2D_Imp::SetPosition(value); }
virtual Vector2DF GetGlobalPosition() override { return CoreObject2D_Imp::GetGlobalPosition(); }
virtual float GetAngle() const override { return CoreObject2D_Imp::GetAngle(); }
virtual void SetAngle(float value) override { return CoreObject2D_Imp::SetAngle(value); }
virtual Vector2DF GetScale() const override { return CoreObject2D_Imp::GetScale(); }
virtual void SetScale(Vector2DF value) override { return CoreObject2D_Imp::SetScale(value); }
virtual void SetParent(CoreObject2D& parent, ChildMode mode) override { return CoreObject2D_Imp::SetParent(parent, mode); }
virtual void ClearParent() override { return CoreObject2D_Imp::ClearParent(); }
virtual Matrix33 GetMatrixToTranslate() override { return CoreObject2D_Imp::GetMatrixToTranslate(); }
virtual Matrix33 GetMatrixToTransform() override { return CoreObject2D_Imp::GetMatrixToTransform(); }
virtual Matrix33 GetParentsMatrix() override { return CoreObject2D_Imp::GetParentsMatrix(); }


void AddChild(CoreObject2D* child, ChildMode mode)
{
	if (child == nullptr || children.find(child) != children.end()) return;
	children.insert(child);
	child->SetParent(*this, mode);
}

void RemoveChild(CoreObject2D* child)
{
	if (child == nullptr) return;
	children.erase(child);
	child->ClearParent();
}