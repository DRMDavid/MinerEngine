#pragma once
#pragma once
#include "Prerequisites.h"
#include "ECS/Component.h"

class DeviceContext;
class Entity;

class HierarchyComponent : public Component
{
public:
	HierarchyComponent() : Component(ComponentType::HIERARCHY) {}
	~HierarchyComponent() = default;

	void
		init() override {}

	void
		update(float) override {}

	void
		render(DeviceContext& deviceContext) override {}

	void
		destroy() override {
		m_children.clear();
		m_parent.reset();
	}

	// API SceneGraph
	void
		setParent(const EU::TSharedPointer<Entity>& parent) {
		m_parent = parent;
	}

	EU::TSharedPointer<Entity>
		getParent() const {
		return m_parent.lock();
	}

	const std::vector<EU::TWeakPointer<Entity>>&
		getChildren() const {
		return m_children;
	}

	void
		addChild(const EU::TSharedPointer<Entity>& child) {
		// evita duplicados
		for (auto& w : m_children)
			if (w.lock() == child) return;

		m_children.push_back(child);
	}

	void
		removeChild(const EU::TSharedPointer<Entity>& child) {
		m_children.erase(
			std::remove_if(m_children.begin(), m_children.end(),
				[&](const EU::TWeakPointer<Entity>& w) { return w.lock() == child; }),
			m_children.end()
		);
	}


private:
	EU::TWeakPointer<Entity> m_parent;
	std::vector<EU::TWeakPointer<Entity>> m_children;
};