{
"solutionorder": ["Storage"],
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
		"Precipitation": {
			"type": "source",
			"includeinoutput": "true",
                        "description": "precipitation time series",
			"ask_user": "true"
		},
		"Area": {
			"type": "value",
			"description": "Catchment area",
			"ask_user": "true"
		},
        	"Slope": {
			"type": "value",
			"description": "Catchment slope in the direction of flow",
			"ask_user": "true"
		},
		"ManningCoeff": {
			"type": "value",
			"description": "Manning's Roughness Coefficient",
			"ask_user": "true"
		},
		"Width": {
			"type": "value",
			"description": "The width of the catchment",
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
			"ask_user": "false"
		},
		"timeseries": {
			"type": "timeseries_prec",		
			"includeinoutput": "true",
			"description": "Predcipitation intensity",
			"ask_user": "true"		
		}
	}
}
