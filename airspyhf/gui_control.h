/**
 *  IW0HDV Extio
 *
 *  Copyright 2017 by Andrea Montefusco IW0HDV
 *
 *  Licensed under GNU General Public License 3.0 or later. 
 *  Some rights reserved. See COPYING, AUTHORS.
 *
 * @license GPL-3.0+ <http://spdx.org/licenses/GPL-3.0+>
 */
 
#if !defined __GUI_CONTROL_H__
#define      __GUI_CONTROL_H__

#include <memory>
class AirSpyHfCtrlGui;
typedef std::shared_ptr<AirSpyHfCtrlGui> PCTRLGUI;

#include "gui.h"
#include "extio_config.h"
#include "extio_airspyhf.h"
#include <config.h>
	
typedef std::tuple<int, int, int, int, int, int> AIRSPYHF_CFG_T;

class AirSpyHfCtrlGui: public Gui {
public:
	AirSpyHfCtrlGui (PEXTPRADIO<EXTIO_BASE_TYPE> &pExr);
	~AirSpyHfCtrlGui ();

	void EnableControls();
	void DisableControls();

	virtual bool OnInit(const GuiEvent& ev);
  virtual bool ComboBoxSelChange(const GuiEvent &ev);
  virtual bool OkPressed(const GuiEvent &ev);
	virtual bool ButtonClick(const GuiEvent &ev);

private:

	PEXTPRADIO<EXTIO_BASE_TYPE> pr_;
	
	enum {
		C_SR     = 0,		// sample rate value
		C_CAL    = 1,		// calibration value in ppb (part per billion)
		C_GPIO_0 = 2,
		C_GPIO_1 = 3,
		C_GPIO_2 = 4,
		C_GPIO_3 = 5,
	} ;
	
	std::unique_ptr < Config<AIRSPYHF_CFG_T> > cfg_;
	
};

#endif
