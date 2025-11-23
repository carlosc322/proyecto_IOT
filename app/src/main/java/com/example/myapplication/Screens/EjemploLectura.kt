package com.example.componentestest.Componentes.Firebase

import androidx.compose.foundation.layout.padding
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import com.example.myapplication.Firebase.ActuatorControl



@Composable
fun EjemploLectura(){
    //Un estado que guarda un objeto SensorData leído desde Firebase.
    var sensor by rememberSaveable  { mutableStateOf<SensorData?>(null) }
    var errorMsg by rememberSaveable { mutableStateOf<String?>(null) }
    var loading by rememberSaveable { mutableStateOf(false) }

    LeerFirebase("sensor_data", SensorData::class.java).apply {
        sensor = first ?: sensor
        loading = second
        errorMsg = third



    }
//ESTO ES LO QUE_CARGA_LOS_DATOS_LO_PODEMOS COPIAR_Y_PEGAR_MODIFICANDO_EL_VALOR
    Text(
        text = "Sensor: ${sensor?.raw_value?:0}",
        modifier = Modifier.padding(16.dp)
    )
    Text(
        text = "Sensor: ${sensor?.device?:"Desconocido"}",
        modifier = Modifier.padding(16.dp)
    )
    Text(
        text = "Ruido: ${sensor?.supera_max?:"Desconocido"}",
        modifier = Modifier.padding(16.dp)
    )
}

