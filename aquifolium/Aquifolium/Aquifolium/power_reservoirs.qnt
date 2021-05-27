{
"Reservoir": {
		"Storage": {
			"type": "balance",		
			"massbalance": "true",
			"flow": "flow",
			"includeinoutput": "true"
		},
		"reservoir_power": {
			"type": "value"
		},
		"reservoir_coeff": {
			"type": "value"
		}
	},
"Reservoir_link": {
		"flow": {
			"type": "expression",		
			"expression": "reservoir_coeff.s*Storage.s^reservoir_power.s",
			"includeinoutput": "true"
		}
	},
"User": {
		"Storage": {
			"type": "balance",		
			"massbalance": "true",
			"flow": "flow",
			"includeinoutput": "true"
		}
	},
"User_flow": {
		"flow": {
			"type": "timeseries",		
			"includeinoutput": "true"
		}
	}

}


