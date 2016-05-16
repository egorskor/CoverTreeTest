#include<iostream>
//#include "CoverTree.cpp"
//#include "Space.cpp"
#include <map>
#include <vector>
#include <set>
using namespace std;

const int MAX_LEVEL = 10;
const int MIN_LEVEL = -10;

using std::string;
class Object
{
	double value;
public:
	Object(double _value)
	{
		value = _value;
	}

	double get_value()
	{
		return value;
	}
};


template <typename dist_t>
class KNNQuery {
	Object* query;
	int k;
	vector<Object*>* objectVector;
	vector<dist_t>* distVector;
public:
	KNNQuery(int _k, Object* _query)
	{
		k = _k;
		query = _query;
		objectVector = new vector<Object*>;
	}

	int get_k()
	{
		return k;
	}

	dist_t DistanceObjLeft(Object* object)
	{
		return abs(query->get_value() - object->get_value());
	}

	void CheckAndAddToResult(const dist_t dist, const Object* object)
	{
		distVector->push_back(dist);
		objectVector->push_back(object);
	}
	void CheckAndAddToResult(const Object* object)
	{
		objectVector->push_back(object);
	}
	void CheckAndAddToResult(const vector<Object*>& bucket) 
	{
		objectVector->insert(objectVector->end(), bucket.begin(), bucket.end());
	}
};
template <typename dist_t>
class Space
{
public:
	Space()
	{}
	int IndexTimeDistance(Object* a, Object* b)
	{
		return abs(a->get_value() - b->get_value());
	}
};
template <typename dist_t>
class CoverTreeMethod
{
private:

	class CoverTreeNode {
	private:
		map<int, vector<CoverTreeNode*>> children;
		//Object* object;
		vector<Object*>* objectVector;
	public:
		CoverTreeNode(Object* _object)
		{
			//object = _object;
			objectVector = new vector<Object*>;
			objectVector->push_back(_object);
		}
		Object* get_object()
		{

			return (*objectVector)[0];
		}
		vector<Object*>* get_all()
		{
			return objectVector;
		}
		vector<CoverTreeNode*> get_children(int level)
		{
			map<int, vector<CoverTreeNode*>>::iterator it = children.find(level);
			if (it == children.end())
			{
				return vector <CoverTreeNode*>();
			}
			return children.at(level);
		}
		void insert_child(int level, CoverTreeNode* child)
		{
			
			children[level].push_back(child);
		}

		void insert_clone(CoverTreeNode* clone)
		{
			objectVector->push_back(clone->get_object());
		}
	};//CoverTreeNode
	CoverTreeNode* root;
public:
	CoverTreeMethod( Space<dist_t>* space,
		const vector<Object*> data)
	{
		if (root == NULL)
		{
			root = new CoverTreeNode(data[0]);
		}
		for (int i = 1; i < data.size(); i++)
		{
			vector<CoverTreeNode*> tmp = { root };
			insert(space, new CoverTreeNode(data[i]), tmp, MAX_LEVEL);
		}
	}

	bool insert( Space<dist_t>* space, CoverTreeNode* point, vector<CoverTreeNode*>& level_points, int level)
	{
		vector<CoverTreeNode*> children =  level_points;
		for (int i = 0; i < level_points.size(); i++)
		{
			vector<CoverTreeNode*> tmp=level_points[i]->get_children(level);
			children.insert(children.end(), tmp.begin(), tmp.end());
		}
		vector<CoverTreeNode*> parents;
		for (int i = 0; i < children.size(); i++)
		{
			float check = pow(2, level);
			if (!(space->IndexTimeDistance(point->get_object(), children[i]->get_object())>=check))//...			
			{
				if (space->IndexTimeDistance(point->get_object(), children[i]->get_object()) == 0)
				{
					children[i]->insert_clone(point);
					return false;
				}
				else {
					CoverTreeNode* tmp = children[i];
					parents.push_back(tmp);
				}
			}
		}
		if (parents.empty())
		{
			return true;
		}
		else
		{
			bool found = insert(space, point, parents, level - 1);
			if (found)
			{
				parents[0]->insert_child(level, point);
				return false;
			}
			else
			{
				return found;
			}
		}
	}


	void Search(KNNQuery<dist_t>* query)
	{
		int k = query->get_k();
		set<pair<dist_t,CoverTreeNode*>> answer = { make_pair(query->DistanceObjLeft(root->get_object()), root)};
		vector<CoverTreeNode*> level_points = { root };
		dist_t minDist = query->DistanceObjLeft(root->get_object());//дл€ работы алгоритма
		dist_t maxDist = minDist;//дл€ составлени€ ответа
		for (int level = MAX_LEVEL; level >= MIN_LEVEL; level--)
		{
			for (int j = 0; j < level_points.size(); j++)
			{
				vector<CoverTreeNode*> children = level_points[j]->get_children(level);
				for (vector<CoverTreeNode*>::iterator i = children.begin(); i != children.end(); i++)
				{
					dist_t dist = query->DistanceObjLeft((*i)->get_object());
					if (dist <= minDist)
					{		
						minDist = dist;
					}
					if (dist < maxDist || answer.size() < k)
					{
						//answer.push_back((dist, children[i]));
						answer.insert(std::make_pair(dist, *i));
						//--answer.end() gives us an iterator to the greatest
						//element of minNodes.
						if (answer.size() > k) answer.erase(--answer.end());
						maxDist = (--answer.end())->first;
					}
					level_points.push_back(*i);
				}
			}
			dist_t check = pow(2, level) + minDist;
			int size = level_points.size();
			for (int i = 0; i<size; i++) 
			{
			   if (query->DistanceObjLeft(level_points[i]->get_object()) > check)//дистанци€ считаетс€ второй раз. ћожетстоило еЄ сохранить?
			   {
				 level_points.erase(level_points.begin() + i);
				 size--; i--;
			   }
			}  
		}
		//set<pair<dist_t, CoverTreeNode*>>::iterator it
		for (set<pair<dist_t, CoverTreeNode*>>::iterator it = answer.begin(); it != answer.end(); it++)//....
		{
			//CoverTreeNode* a= answer[it].second;
			vector<Object*>* objectVector=it->second->get_all();
			query->CheckAndAddToResult(*objectVector);
		}
	}

	/*void Search(KNNQuery<dist_t>* query)
	{

	}*/
};//CoverTree

int main()
{
	 KNNQuery<int>* query=new KNNQuery<int> (2, new Object(5));//второй аргумент точка, к которой ищем ближайших соседей; первый аргумент - их количество
	 Space<int>* fakespace = new Space<int>();
	 vector<Object*> data = {new Object(1), new Object(5), new Object(6),  new Object(6), new Object(7), new Object(46), new Object(3), new Object(2) };
	 CoverTreeMethod<int> a(fakespace, data);
	 a.Search(query);
	 CoverTreeMethod<int> b =  a;
	
}