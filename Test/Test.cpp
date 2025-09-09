#include <iostream>
#include <unordered_set>
#include <set>

struct Ref
{
	size_t id;

	bool operator==(const Ref& other) const
	{
		return id == other.id;
	}

	bool operator<(const Ref& other) const
	{
		return id < other.id;
	}
};

namespace std
{
	template<>
	struct hash<Ref>
	{
		size_t operator()(const Ref& ref) const
		{
			return ref.id;
		}
	};
}

//
struct A
{
	size_t id;
	std::unordered_set<Ref> children;
};

int main()
{
	A a{ 1 };
	if (not a.children.contains(Ref(3)))
	{
		a.children.insert(Ref{ 3 });
	}
}