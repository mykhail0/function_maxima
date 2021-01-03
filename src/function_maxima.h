#ifndef FUNCTION_MAXIMA_H
#define FUNCTION_MAXIMA_H

/*
class MapIterator {
  private:
  	iterator z mapy
  operator*() {
    
  }
}

pair<A, V> --> point_type
*/



template <typename A, typename V>
class FunctionMaxima {
  private:
  	using Point = pair<A, V>;
  
    class point_type {
      private:
    		shared_ptr<Point> ptr;

      	friend point_type FunctionMaxima::make_point(A, V);
        point_type(Point* ptr) : ptr(ptr) = default;
      public:
        // Zwraca argument funkcji.
        A const& arg() const {
          	return ptr.get()->first;
        }

        // Zwraca wartość funkcji w tym punkcie.
        V const& value() const {
          	return ptr.get()->newsecond;
        }
    };


  MS 1) klasa do testowania exception
  JS 1) comparator set
  JS 1) destruktor
  JS 1) point_type, make_point

    // copy and swap idiom
    // non throw swap idiom
    constr copy tmp(rhs)
    non throw swap (this, tmp)
  MS 2) konstruktor kopia, operator= --- strong guarantee

  erase, set_value
  find, value_at


  /// 1111
  point_type make_point(A,V);
  set<point_type> points;
  set<point_type> mx_points;

  public:
  	using mx_iterator = set::const_iterator;
  	using iterator = set::const_iterator;
  	using size_type = set::size_type;

    FunctionMaxima() = default;
    FunctionMaxima(const FunctionMaxima&);
		FunctionMaxima &operator=(const FunctionMaxima&);
    ~FunctionMaxima();

		iterator begin() const {
      return points.begin();
    }

  	iterator end() const {
      return points.end();
    }

  	iterator find(A const& x) const {
      // TODO strong exception
      try {
        point_type pt = make_point(x, static_cast<V>(nullptr));
      }
    	return points.find(pt);
    }

		mx_iterator mx_begin() const {
      return mx_points.begin();
    }
  
  	mx_iterator mx_end() const {
     return  mx_points.end();
    }
  
  set value mutator

    erase mutator

    value at inspector
    
    size()
};

#endif /* FUNCTION_MAXIMA_H */