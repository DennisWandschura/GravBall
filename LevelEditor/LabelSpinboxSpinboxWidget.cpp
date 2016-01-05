#include "LabelSpinboxSpinboxWidget.h"

LabelSpinboxSpinboxWidget::LabelSpinboxSpinboxWidget(QWidget* parent, const char* text)
	:QWidget(parent),
	m_label(nullptr),
	m_spinbox_x(nullptr),
	m_spinbox_y(nullptr)
{
	resize(60 + 75 + 5 + 75 + 5, 25);
	m_label = new QLabel(this);
	m_label->setText(text);
	m_label->resize(60, 20);

	m_spinbox_x = new QDoubleSpinBox(this);
	m_spinbox_y = new QDoubleSpinBox(this);

	m_spinbox_x->resize(75, 20);
	m_spinbox_y->resize(75, 20);

	m_spinbox_x->move(60, 0);
	m_spinbox_y->move(60 + 75 + 5, 0);

	m_spinbox_x->setMaximum(9999.0);
	m_spinbox_x->setMinimum(-9999.0);

	m_spinbox_y->setMaximum(9999.0);
	m_spinbox_y->setMinimum(-9999.0);
}

void LabelSpinboxSpinboxWidget::setMinMax(const vx::float2 &v0, const vx::float2 &v1)
{
	m_spinbox_x->setMinimum(v0.x);
	m_spinbox_x->setMaximum(v0.y);

	m_spinbox_y->setMinimum(v1.x);
	m_spinbox_y->setMaximum(v1.y);
}

void LabelSpinboxSpinboxWidget::setValues(f32 x, f32 y)
{
	m_spinbox_x->setValue(x);
	m_spinbox_y->setValue(y);
}

vx::float2 LabelSpinboxSpinboxWidget::getValues() const
{
	return vx::float2(m_spinbox_x->value(), m_spinbox_y->value());
}