#pragma once
#include "Prerequisites.h"

class Entity;
class DeviceContext;

class
	SceneGraph {
public:
	SceneGraph() = default;
	~SceneGraph() = default;

	void
		init();

	void
		addEntity(const EU::TSharedPointer<Entity>& e);  // registra en el grafo

	void
		attach(const EU::TSharedPointer<Entity>& child,
			const EU::TSharedPointer<Entity>& parent);

	void
		detach(const EU::TSharedPointer<Entity>& child);

	void
		update(float deltaTime, DeviceContext& deviceContext);

	void
		render(DeviceContext& deviceContext);

private:
	void
		updateWorldRecursive(const EU::TSharedPointer<Entity>& node,
			const XMMATRIX& parentWorld);

	bool
		isRoot(const EU::TSharedPointer<Entity>& e) const;

private:
	std::vector<EU::TSharedPointer<Entity>> m_entities;
}; 
