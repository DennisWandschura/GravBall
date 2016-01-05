#pragma once

#include "qwidget.h"
#include "qlabel.h"
#include "qspinbox.h"

class LabelSpinboxWidget : public QWidget
{
	QLabel* m_label;
	QDoubleSpinBox* m_spinbox;

public:
	LabelSpinboxWidget(QWidget* parent, const char* text);

	~LabelSpinboxWidget()
	{
		delete(m_spinbox);
		delete(m_label);
	}

	void setMinMax(float min, float max);

	void setValue(float v);

	float getValue() const;

	QDoubleSpinBox* getSpinBox() { return m_spinbox; }
};