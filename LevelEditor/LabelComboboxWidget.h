#pragma once

#include "qwidget.h"
#include "qlabel.h"
#include "qcombobox.h"

class LabelComboboxWidget : public QWidget
{
	QLabel* m_label;
	QComboBox* m_combobox;

public:
	LabelComboboxWidget(QWidget* parent, const char* text);

	~LabelComboboxWidget()
	{
		delete(m_combobox);
		delete(m_label);
	}

	void addItem(const QString &text);
	void setSelected(int i);
	int getSelectedIndex() const;
	const QString& getSelectedText() const;

	QComboBox* getQComboBox() { return m_combobox; }
};