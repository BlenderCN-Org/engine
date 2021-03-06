/**
 * @file
 */

#pragma once

#include "ui/turbobadger/Widget.h"

class LayerItem: public tb::TBGenericStringItem {
public:
	LayerItem(int layerId, const char *name, bool visible = true) :
		tb::TBGenericStringItem(name, TBIDC(name)), _layerId(layerId), _visible(visible) {
	}
	inline void setVisible(bool visible) {
		_visible = visible;
	}
	inline bool visible() const {
		return _visible;
	}
	inline int layerId() const {
		return _layerId;
	}
private:
	int _layerId;
	bool _visible;
};

class LayerItemSource: public tb::TBSelectItemSourceList<LayerItem> {
public:
	tb::TBWidget *createItemWidget(int index, tb::TBSelectItemViewer *viewer) override;
	int getItemIdForLayerId(int layerId) const;
	LayerItem* getItemForLayerId(int layerId) const;
};

class LayerWidget: public tb::TBWidget, public SceneListener {
private:
	using Super = tb::TBWidget;
public:
	TBOBJECT_SUBCLASS(LayerWidget, tb::TBWidget);

	LayerWidget();
	~LayerWidget();
	bool onEvent(const tb::TBWidgetEvent &ev) override;

	void onLayerHide(int layerId) override;
	void onLayerShow(int layerId) override;
	void onActiveLayerChanged(int old, int active) override;
	void onLayerAdded(int layerId, const Layer& layer) override;
	void onLayerDeleted(int layerId) override;
private:
	tb::TBSelectList *_list;
	LayerItemSource _source;
};

UIWIDGET_FACTORY(LayerWidget, tb::TBValue::TYPE_INT, tb::WIDGET_Z_TOP)
