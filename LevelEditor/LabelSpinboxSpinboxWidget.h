#pragma once

#include <vxLib/math/Vector.h>
#include "qwidget.h"
#include "qlabel.h"
#include "qspinbox.h"

class LabelSpinboxSpinboxWidget : public QWidget
{
	QLabel* m_label;
	QDoubleSpinBox* m_spinbox_x;
	QDoubleSpinBox* m_spinbox_y;

public:
	LabelSpinboxSpinboxWidget(QWidget* parent, const char* text);

	~LabelSpinboxSpinboxWidget()
	{
		delete(m_spinbox_y);
		delete(m_spinbox_x);
		delete(m_label);
	}

	void setMinMax(const vx::float2 &v0, const vx::float2 &v1);
	void setValues(f32 x, f32 y);

	vx::float2 getValues() const;

	QDoubleSpinBox* getSpinBox_x() { return m_spinbox_x; }
	QDoubleSpinBox* getSpinBox_y() { return m_spinbox_y; }
};