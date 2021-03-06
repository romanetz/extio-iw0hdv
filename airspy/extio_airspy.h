/**
 *  IW0HDV Extio
 *
 *  Copyright 2015 by Andrea Montefusco IW0HDV
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */

 #if !defined	__EXTIO_AIRSPY_H__
#define			__EXTIO_AIRSPY_H__

#include "extio_config.h"

/*
 * class ExtioDataConversion
 *
 * helper class for buffering and conversion of AIRSPY samples to Extio formats 
 *
 */
template < typename ST >
class ExtioDataConversion {
public:
	ExtioDataConversion (int _ns):ns(_ns) { pb = new unsigned char[2 * ns * sizeof(typename ST::sample_type)]; }
	~ExtioDataConversion() { delete [] pb;  }
	int getNs() { return ns; }

	unsigned char *pb;
	int ns;

	int convert_iq_from_rx (AirSpyRxIQSample *i, AirSpyRxIQSample *q, ExtIO_float32)
	{
		float *p = (float *)pb;
		for (int n = 0; n < ns; ++n)
			*p++ = i->float_32(),
			*p++ = q->float_32(),
			++i,
			++q;
		return 0;
	}
};


#include <stdint.h>
#include "log.h"
#include "airspy.h"

template <typename ST>
class ExtioAirSpyRadio : public ExtioDataConversion<ST>, public AirSpyRadio
{
public:
	ExtioAirSpyRadio(int ns, EXTIO_RX_CALLBACK *pCb): 
		ExtioDataConversion<ST>(ns), 
		AirSpyRadio (),
		cnt(0), 
		pExtioCallback(pCb),
		ns_(ns)
	{  }

	virtual ~ExtioAirSpyRadio() { LOGT("%s\n", "~ExtioAirSpyRadio()"); }


	// called from the receiving context when the buffer is full
	int send_iq_from_rx_to_dsp (AirSpyRxIQSample *i, AirSpyRxIQSample *q, int ns)
	{
		// convert and copy raw I/Q data from two buffers in native AirSpy format to ExtIO format
		this->convert_iq_from_rx(i, q, ST()); // Extio type selection is done at compile type !

		//if ((cnt % 1024) == 0) { LOGT("---------- buflen: %d\n", ns); }

		// send data to client SDR program
		(*pExtioCallback) (ns, 0, 0., (int *) this->pb );

		cnt++; // housekeeping: increase internal buffers counter
		return 0;
	}
	
	//
	// called from AIRSPY data callback
	// this method is declared in AirSpyRadio class
	// The buffer received from hardware contains 32-bit floating point IQ samples (8 bytes per sample)
	//
	int data_available (void *buf, int buf_size)
	{	
		float	*sbuf	= (float *)buf;
		int nSamples	= buf_size/(sizeof(float)*2); // 4*2 = 8 bytes for sample, as per AirSpy USB data stream format

		// Transform data from linear interleaved native Perseus hardware USB format
		// to two buffers, one for I and the other one for Q
		AirSpyRxIQSample i_buf [ns_]; // * sizeof(typename ST::sample_type)];
		AirSpyRxIQSample q_buf [ns_]; // * sizeof(typename ST::sample_type)];
		
		for (int k=0; k < nSamples; k++) {
			i_buf[k].f = *sbuf++ ;
			q_buf[k].f = *sbuf++ ;
		}
		// send to client SDR program
		this->send_iq_from_rx_to_dsp (i_buf, q_buf, nSamples);
	
		return 0;
	}
	/*
	 * method used in order to force a sample rate change, usually as
	 * per user input on GUI
	 */
	void setSampleRateHW(int new_sr)
	{
		this->set_sample_rate(new_sr);

		/* 100
			This status value indicates that a sampling frequency change has taken place,
			either by a hardware action, or by an interaction of the user with the DLL GUI.
			When the main program receives this status, it calls immediately after
			the GetHWSR() API to know the new sampling rate.
			We are calling the callback only for the first instance (otherwise Studio 1 is looping on Start/Stop cycle - TBI).
		*/
		if (*pExtioCallback && (Dll::GetInstanceNumber()==1)) {
			LOGT("new sample rate: %d\n", new_sr);
			(*pExtioCallback) (-1, 100, 0., 0);
		}
	
	}
	
	int startHW (int buf_size) 
	{ 
		return this->start(buf_size); 
	}
	
protected:
	int cnt;
	EXTIO_RX_CALLBACK *pExtioCallback;
	int ns_;
};

#include <memory>
template <class ST> using PEXTPRADIO = std::shared_ptr < ExtioAirSpyRadio<ST> > ;

#include "gui_splashscreen.h"
#include "gui_control.h"

class ExtIODll : public Extio {
public:
	
	virtual bool InitHW(char *name, char *model, int& extio_type);
	virtual bool OpenHW(void);
	virtual void CloseHW(void);
	
	virtual int  StartHW(long freq);
	virtual void StopHW(void);
	
	virtual int SetHWLO(long LOfreq);
	virtual long GetHWLO(void);
	
	virtual long GetHWSR(void);
	virtual int GetStatus(void);
	
	virtual void ShowGUI(void);
	virtual void HideGUI(void);
	
	virtual const char *name() { return "AIRSPY-IW0HDV"; }

private:

	// GUI
	PSPLASH  pSplash;
	PCTRLGUI pGui;

    // Radio
    PEXTPRADIO<EXTIO_BASE_TYPE> pExr;

	// copy ctor and assignment op deleted: this is a singleton global object
    ExtIODll (const ExtIODll &) = delete;
    ExtIODll & operator=(const ExtIODll &) = delete ;

    // static object that guarantees one (and one only) instance is generated at the global
	// level, so that C++ runtime builds it , calling the empty c'tor
	// the trick here is that the following static member is an object of this _same_ class
	// so it has access to c'tor even if it is kept private
	// see http://accu.org/index.php/journals/1328 by Alexander Nasonov
	//
    static ExtIODll singleton;

	ExtIODll ();
	virtual ~ExtIODll ();
};

#endif
