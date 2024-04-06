#pragma once


class Bakery
{
public:
	int n;
	// ������ �ִ� ���� 8�� ����
	bool m_flag[8];
	int m_label[8];

	Bakery(int n) : n(n) {
		for (int i = 0; i < n; ++i)
		{
			m_flag[i] = false;
			m_label[i] = 0;
		}
	}

	Bakery() { }

	int Max()
	{
		int max = 0;
		for (int i = 0; i < n; ++i)
		{
			if (m_label[i] > max)	max = m_label[i];
		}
		return max;
	}

	void Lock(int thread_id)
	{
		int i = thread_id;
		m_flag[i] = false;
		m_label[i] = Max() + 1;
		for (int j = 0; j < n; j++)
		{
			if (j == i) continue;			// �ڽ� �ǳʶٱ�
			while (m_flag[j]) {}			// �ٸ� �����尡 ��ȣ ���� ������ ����
			
			// ���� ���� ������ j�� �� �۰ų�(�켱������ ���ų�), 
			// ����ȣ(label)�� �����鼭 ���� ���� �������� id�� �� ������(�켱������ ������) ���
			while (m_label[j] != 0 && (m_label[j] < m_label[i] || (m_label[j] == m_label[i] && j < i)) ) {}	
		}
	}

	void UnLock(int thread_id)
	{
		m_flag[thread_id] = false;
	}
};
