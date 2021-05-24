#pragma once

#include <stack>
#include <utility>

template <typename T> class AVLTree
{
public:
	struct node
	{
		T data;
		char weight;
		node* next[2];
		node() : data{}, weight{}, next {} {}
		template <typename U> explicit node(U&& x) : data{ std::forward<U>(x) }, weight{}, next {} {}
	};
private:
	// Node address and direction to next node: 0 = left, 1 = right
	using path_type = std::pair<node**, int>;
	std::stack<path_type> path_history;
	node* head;

	int _select(const node* n, const T& x)
	{
		return !(x < n->data);
	}

	/*	Return the address of the node with value 'x',
		or the node which should have it.
		Side-effect:
			The path from the start to the node before it
			is stored in 'path_history'. */
	node** _find(node** handle, const T& x)
	{
		node* current = *handle;
		if (current == nullptr || current->data == x)
			return handle;

		int which = _select(current, x);
		path_history.push(path_type{ handle, which });

		while (current->next[which] && current->next[which]->data != x)
		{
			handle = &current->next[which];
			current = *handle;
			which = _select(current, x);
			path_history.push(path_type{ handle, which });
		}

		return &current->next[which];
	}

	void _delete(node** handle)
	{
		node* n = *handle;
		if (n->next[0] && n->next[1])
		{
			path_history.push(path_type{ handle, 1 });
			handle = &(*handle)->next[1];

			while ((*handle)->next[0])
			{
				path_history.push(path_type{ handle, 0 });
				handle = &(*handle)->next[0];
			}

			n->data = (*handle)->data;
			n = *handle;
		}
		node* temp = n->next[0] ? n->next[0] : n->next[1];
		delete n;
		*handle = temp;
	}

	void _clear()
	{
		if (head == nullptr)
			return;

		std::stack<node*> destr_stack{};
		destr_stack.push(head);
		while (!destr_stack.empty())
		{
			node* top = destr_stack.top();
			destr_stack.pop();

			if (top->next[0])
				destr_stack.push(top->next[0]);

			if (top->next[1])
				destr_stack.push(top->next[1]);

			delete top;
		}
	}

	void _rotate_left(node** handle)
	{
		node* n = *handle;
		*handle = n->next[1];
		n->next[1] = (*handle)->next[0];
		(*handle)->next[0] = n;
	}

	void _rotate_right(node** handle)
	{
		node* n = *handle;
		*handle = n->next[0];
		n->next[0] = (*handle)->next[1];
		(*handle)->next[1] = n;
	}

	/*	diff = 1 if insert was made, diff = -1 if delete was made */
	void _balance(char diff)
	{
		path_type current;
		node* n;

		while (!path_history.empty())
		{
			current = path_history.top(); path_history.pop();
			n = *current.first;

			n->weight += current.second ? diff : -diff;
			
			if (n->weight == 2 || n->weight == -2)
			{
				int which = n->weight > 1 ? 1 : 0;
				int next_weight = n->next[which]->weight;
				if (which == 0)	// n->weight = -2
				{
					if (next_weight == 1)
					{
						int next_next_weight = n->next[0]->next[1]->weight;
						_rotate_left(&n->next[0]);
						_rotate_right(current.first);
						(*current.first)->weight = 0;
						(*current.first)->next[0]->weight = 0;
						(*current.first)->next[1]->weight = 0;
						if (next_next_weight == -1)
							(*current.first)->next[1]->weight = 1;
						if (next_next_weight == 1)
							(*current.first)->next[0]->weight = -1;
					}
					else if (next_weight == 0)
					{
						_rotate_right(current.first);
						(*current.first)->weight = 1;
						n->weight = -1;
					}
					else
					{
						_rotate_right(current.first);
						(*current.first)->weight = 0;
						n->weight = 0;
					}
				}
				else
				{
					if (next_weight == -1)
					{
						int next_next_weight = n->next[1]->next[0]->weight;
						_rotate_right(&n->next[1]);
						_rotate_left(current.first);
						(*current.first)->weight = 0;
						(*current.first)->next[0]->weight = 0;
						(*current.first)->next[1]->weight = 0;
						if (next_next_weight == 1)
							(*current.first)->next[0]->weight = -1;
						if (next_next_weight == -1)
							(*current.first)->next[1]->weight = 1;
					}
					else if (next_weight == 0)
					{
						_rotate_left(current.first);
						(*current.first)->weight = -1;
						n->weight = 1;
					}
					else
					{
						_rotate_left(current.first);
						(*current.first)->weight = 0;
						n->weight = 0;
					}
				}
				n = *current.first;
			}

			if ((diff == 1 && n->weight == 0) || (diff == -1 && n->weight != 0))
			{
				while (!path_history.empty())
					path_history.pop();
				return;
			}
		}
	}

	int _check_balanced(node* n)
	{
		if (n == nullptr)
			return 0;

		int left_height = _check_balanced(n->next[0]);
		int right_height = _check_balanced(n->next[1]);

		if (left_height == -1 || right_height == -1)
			return -1;

		if ((left_height > right_height && n->weight != -1) ||
			(right_height > left_height && n->weight != 1) ||
			(left_height == right_height && n->weight != 0))
			return -1;

		return (left_height > right_height ? left_height : right_height) + 1;
	}
public:
	AVLTree() : head{ nullptr } {}
	AVLTree(const AVLTree&) = delete;
	~AVLTree() { _clear(); }
	AVLTree& operator= (const AVLTree&) = delete;

	template <typename U> void Insert(U&& x)
	{
		if (head == nullptr)
		{
			head = new node{ std::forward<U>(x) };
			return;
		}

		node** current = _find(&head, x);

		while (*current)
		{
			path_history.push(path_type{ current, 1 });
			current = _find(&(*current)->next[1], x);
		}

		*current = new node{ std::forward<U>(x) };
		_balance(1);
	}

	void Delete(const T& x)
	{
		node** result = _find(&head, x);

		if (*result)
		{
			_delete(result);
			_balance(-1);
		}
		else
		{
			while (!path_history.empty())
				path_history.pop();
		}
	}

	const node* Find(const T& x)
	{
		node* current = head;

		while (current && current->data != x)
			current = current->next[_select(current, x)];

		return current;
	}

	void Clear()
	{
		_clear();
		head = nullptr;
	}

	bool CheckBalanced()
	{
		return _check_balanced(head) != -1;
	}
};