#ifndef __Navigation_list_h__
#define __Navigation_list_h__

#include "IList.h"
#include <sh_vector.h>

namespace Navigation {
	template <class T> class List : public IList<T> {
	public:
		List() {
			this->items = new SourceHook::CVector<T>();
		}

		~List() {
			delete this->items;
		}

		void Destroy() {
			delete this;
		}

		bool Insert(T item, unsigned int index) {
			size_t size = this->items->size();

			if(index < 0 || index > size) {
				return false;
			}

			this->items->insert(this->items->iterAt(index), item);
			return true;
		}

		void Append(T item) {
			this->items->insert(this->items->end(), item);
		}

		void Prepend(T item) {
			this->items->insert(this->items->begin(), item);
		}

		T At(unsigned int index) {
			return this->items->at(index);
		}

		size_t Size() {
			return this->items->size();
		}

		void Erase(unsigned int index) {
			this->items->erase(this->items->iterAt(index));
		}

		unsigned int Find(T item) {
			size_t size = this->items->size();

			for(unsigned int i = 0; i < size; i++) {
				if(this->items->at(i) != item) {
					continue;
				}

				return i;
			}

			return -1;
		}

	private:
		SourceHook::CVector<T> *items;
	};
}

#endif