{
"solutionorder": ["Storage","Pollutant"],
"Catchment": {
		"type": "block",
		"icon": {
			"filename": "catchment.png" 
		},
		"Storage": {
			"type": "balance",		
			"massbalance": "true",
			"flow": "flow",
			"inflow": "Precipitation",
			"includeinoutput": "true",
			"description": "Initial Storage",
			"ask_user": "true"
		},
		"Pollutant": {
			"type": "balance",		
			"massbalance": "true",
			"flow": "mass_flow",
			"includeinoutput": "true",
			"description": "Initial Pollutant Mass",
			"ask_user": "true"
		},
		"Precipitation": {
			"type": "source",
			"description": "precipitation time series",
			"ask_user": "true"
		},
		"Area": {
			"type": "value",
			"description": "Catchment area",
			"ask_user": "true"
		}	
},
"Precipitation": {
		"type": "source",
		"icon": {
			"filename": "cloud.png"
		},
		"coefficient": {
			"type": "expression",		
			"expression": "Area",
			"ask_user": "true"
		},
		"timeseries": {
			"type": "timeseries",		
			"includeinoutput": "true",
			"description": "Predcipitation intensity",
			"ask_user": "true"		
		}
},
"Reservoir": {
		"type": "block",
		"icon": {
			"filename": "reservoir.png" 
		},
		"Storage": {
			"type": "balance",		
			"massbalance": "true",
			"flow": "flow",
			"inflow": "inflow",
			"includeinoutput": "true",
			"description": "Initial Storage",
			"ask_user": "true"
		},
		"Pollutant": {
			"type": "balance",		
			"massbalance": "true",
			"flow": "mass_flow",
			"includeinoutput": "true",
			"description": "Initial Pollutant Mass",
			"ask_user": "true"
		},
		"reservoir_power": {
			"type": "value",
			"default": "2",
			"description": "Reservoir rating curve power",
			"ask_user": "true"
		},
		"reservoir_coeff": {
			"type": "value",
			"default": "10",
			"description": "Reservoir rating curve coefficient",
			"ask_user": "true"
		},
		"inflow": {
			"type": "timeseries",		
			"includeinoutput": "true",
			"applylimit": "true",
			"description": "Flow rate time series",
			"ask_user": "true"
		}
	},
"Reservoir_link": {
		"type": "link",
		"icon": {
			"filename": "reservoir_link.png" 
		},
		"flow": {
			"type": "expression",		
			"expression": "reservoir_coeff.s*Storage.s^reservoir_power.s",
			"includeinoutput": "true",
			"applylimit": "true",
			"ask_user": "true"
		},
		"mass_flow": {
			"type": "expression",		
			"expression": "_ups(flow;(Pollutant/Storage))",
			"includeinoutput": "true",
			"applylimit": "true",
			"ask_user": "true"
		}
	},
"Reservoir_link_rule": {
		"type": "link",
		"icon": {
			"filename": "reservoir_link.png" 
		},
		"flow": {
			"type": "rule",		
			"rule": 
			{	
				"Storage.s<S_min":"Q_min",
				"S_min<Storage.s<S_max":"Q_min+((Storage.s-S_min)/(S_max-S_min))*(Q_max-Q_min)",
				"Storage.s>S_max":"Q_max"
			},
			"includeinoutput": "true",
			"applylimit": "true",
			"ask_user": "false"
		},
		"mass_flow": {
			"type": "expression",		
			"expression": "_ups(flow;(Pollutant/Storage))",
			"includeinoutput": "true",
			"applylimit": "true",
			"ask_user": "true"
		},
		"Q_min" : {
			"type": "value",
			"default": "10",
			"description": "Minimum flow",
			"ask_user": "true"
		},
		"Q_max" : {
			"type": "value",
			"default": "100",
			"description": "Maximum flow",
			"ask_user": "true"
		},
		"S_min" : {
			"type": "value",
			"default": "10",
			"description": "Minimum storage",
			"ask_user": "true"
		},
		"S_max" : {
			"type": "value",
			"default": "100",
			"description": "Maximum storage",
			"ask_user": "true"
		}
	},
"User": {
		"type": "block",
		"icon": {
			"filename": "farm.png" 
		},
		"Storage": {
			"type": "balance",		
			"massbalance": "true",
			"flow": "flow",
			"includeinoutput": "true",
			"description": "Initial Storage",
			"ask_user": "true"
		},
      "Pollutant": {
			"type": "balance",		
			"massbalance": "true",
			"flow": "mass_flow",
			"includeinoutput": "true",
			"description": "Initial Pollutant Mass",
			"ask_user": "true"
		}
	},
"User_flow": {
		"type": "link",
		"icon": {
			"filename": "farm_link.png" 
		},
		"flow": {
			"type": "timeseries",		
			"includeinoutput": "true",
			"applylimit": "true",
			"description": "Flow rate time series",
			"ask_user": "true"
		},
		"mass_flow": {
			"type": "expression",		
			"expression": "_ups(flow;(Pollutant/Storage))",
			"includeinoutput": "true",
			"applylimit": "true",
			"ask_user": "true"
		}
	}
}
