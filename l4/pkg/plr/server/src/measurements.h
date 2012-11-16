#pragma once

#include <cassert>
#include <climits>
#include <utility>

/*
 * Namespace containing classes regarding measurements and
 * sensors etc.
 */
namespace Measurements
{

/**************************************************
                  SENSOR DATA
 **************************************************/

/*
 * A basic data store is there to store data of a given type.
 */
template <typename TYPE>
class Datastore
{
	public:
		typedef TYPE Type;
};

/*
 * Datastore to keep a single value, such as a count
 */
template <typename TYPE>
class SingleValueDatastore : public Datastore<TYPE>
{
	TYPE _val;

	public:
		SingleValueDatastore(TYPE init)
			: _val(init)
		{ }

		virtual TYPE const value() const { return _val; }
		virtual void set(TYPE value)     { _val = value; }
};


/*
 * Datastore to keep an array of COUNT values of the same TYPE.
 */
template <typename TYPE, int COUNT>
class ArrayDatastore : public Datastore<TYPE>
{
	TYPE _values[COUNT];

	public:
		ArrayDatastore(TYPE init)
		{
			for (unsigned i = 0; i < COUNT; ++i)
				_values[i] = init;
		}

	virtual TYPE const value(unsigned idx) const { return _values[idx]; }

	virtual void set(unsigned idx, TYPE value)   {
		assert(idx < COUNT);
		_values[idx] = value;
	}
};



/**************************************************
               DATA ACCESS STRATEGIES

 Access strategies come as read access and write
 access stratgies. They are combined by the generic
 sensor class to implement a specific sensor.
 **************************************************/


/*
 * Simple read/write strategy.
 */
template <typename T, typename DS>
class ReadWriteStrategy
{
	public:
		void operator () (DS& ds, T v)
		{ ds.set(v); }

		typedef T ReadType;
		T const operator () (DS& ds)
		{ return ds.value(); }

		void reset(DS& ds)
		{ ds.set(T()); }
};


/*
 * Strategy implementing a MinMax Sensor.
 */
template <typename T, typename DS>
class MinMaxStrategy
{
	public:
		enum {MIN = 0, MAX = 1};

		void operator () (DS& ds, T v)
		{
			if (v < ds.value(MIN))
				ds.set(MIN, v);
			if (v > ds.value(MAX))
				ds.set(MAX, v);
		}

		typedef std::pair<T, T> ReadType;
		ReadType const operator() (DS& ds)
		{
			return std::pair<T, T>(ds.value(MIN), ds.value(MAX));
		}


		void reset(DS& ds)
		{
			ds.set(MIN, INT_MAX);
			ds.set(MAX, INT_MIN);
		}
};


/*
 * Strategy implementing an average sensor.
 */
template <typename T, typename DS>
class AverageStrategy
{
	public:
		enum { SUM = 0, COUNT = 1 };

		void operator () (DS& ds, T v)
		{
			T sum = ds.value(SUM);
			T cnt = ds.value(COUNT);
			sum += v;
			cnt += 1;
			ds.set(SUM, sum);
			ds.set(COUNT, cnt);
		}

		typedef double ReadType;
		ReadType operator () (DS& ds)
		{
			if (ds.value(COUNT) == 0) return 0.0;

			return static_cast<ReadType const>(ds.value(SUM)) / ds.value(COUNT);
		}


		void reset(DS& ds)
		{
			ds.set(SUM, 0);
			ds.set(COUNT, 0);
		}
};

/*
 * Abstract interface for a sensor. A sensor is
 * used to observe events with a given value and
 * act upon them depending on a strategy.
 */
template <typename DATASTORE,
		  typename STRATEGY>
class SensorInstance
{
	protected:
		DATASTORE _ds;

	public:
		typedef typename DATASTORE::Type    StoredType;
		typedef typename STRATEGY::ReadType ReadType;

		SensorInstance(StoredType t) : _ds(t) { reset(); }

		/*
		 * Show _one_ event to the sensor and perform
		 * handling based on the given strategy.
		 */
		virtual void event(StoredType t) { STRATEGY()(_ds, t); } 

		/*
		 * Read the current sensor value, again depending
		 * on the strategie's understanding of 'read'.
		 */
		virtual ReadType const read() { return STRATEGY()(_ds); }

		/*
		 * Reset the sensor to a predefined value.
		 */
		virtual void reset() { STRATEGY().reset(_ds); };
};

/**************************************************
               USEFUL SENSORS
 **************************************************/

/*
 * Sensor keeping track of minimum and maximum values
 * as they are generated.
 */
class MinMaxSensor
	: public SensorInstance<ArrayDatastore<int, 2>,
					MinMaxStrategy<int, ArrayDatastore<int, 2> > >
{
	public:
		MinMaxSensor()
			: SensorInstance(0)
		{ }
};


/*
 * Sensor computing an average of all seen values.
 */
class AverageSensor
	: public SensorInstance<ArrayDatastore<int, 2>,
					AverageStrategy<int, ArrayDatastore<int, 2> > >
{
	public:
		AverageSensor()
			: SensorInstance(0)
		{ }
};

}
