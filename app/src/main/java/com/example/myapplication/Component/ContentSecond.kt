package com.example.myapplication.Component

import androidx.annotation.ContentView
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.offset
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions

import androidx.compose.runtime.Composable
import androidx.compose.runtime.getValue
import androidx.compose.runtime.mutableStateOf
import androidx.compose.runtime.saveable.rememberSaveable
import androidx.compose.runtime.setValue
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.material3.Button
import androidx.compose.material3.ButtonDefaults
import androidx.compose.material3.OutlinedTextField
import androidx.compose.material3.Text
import androidx.compose.runtime.LaunchedEffect



import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType


import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.componentestest.Componentes.Firebase.LeerFirebase
import com.example.componentestest.Componentes.Firebase.SensorData
import com.example.componentestest.Componentes.Firebase.escribirFirebase
import com.example.myapplication.Firebase.ActuatorControl


// 🔹 Helpers para leer desde Firebase
@Composable
fun DatosActuator(): Triple<ActuatorControl?, Boolean, String?> {
    return LeerFirebase("ActuatorControl", ActuatorControl::class.java)
}

@Composable
fun DatosSensor(): Triple<SensorData?, Boolean, String?> {
    return LeerFirebase("SensorData", SensorData::class.java)
}

/* ---------------------------------------------------------
   1) MOSTRAR TEMPERATURA
--------------------------------------------------------- */
@Composable
fun MostrarTemp() {
    val (sensor, loading, error) = DatosSensor()

    Box(
        modifier = Modifier
            .width(280.dp)
            .height(140.dp)
            .clip(RoundedCornerShape(5.dp))
            .background(Color.Blue)
            .padding(8.dp) // un pequeño padding interno
    ) {
        when {
            loading -> {
                // Mientras carga
                Text(
                    text = "Cargando temperatura...",
                    modifier = Modifier.align(Alignment.Center),
                    color = Color.White
                )
            }

            error != null -> {
                // Si hubo error
                Text(
                    text = "Error al leer: $error",
                    modifier = Modifier.align(Alignment.Center),
                    color = Color.Yellow,
                    fontSize = 12.sp
                )
            }

            else -> {
                // Cuando ya tenemos el dato del sensor
                Column(
                    modifier = Modifier.fillMaxSize()
                ) {
                    // Título
                    Box(
                        modifier = Modifier
                            .fillMaxWidth()
                            .height(35.dp)
                            .background(Color.Yellow),
                        contentAlignment = Alignment.CenterStart
                    ) {
                        Text(
                            text = "Temperatura",
                            modifier = Modifier.padding(horizontal = 10.dp),
                            fontSize = 15.sp,
                            fontWeight = FontWeight.Medium
                        )
                    }

                    // Valor del sensor en el centro
                    Box(
                        modifier = Modifier
                            .fillMaxSize(),
                        contentAlignment = Alignment.Center
                    ) {
                        Text(
                            text = "Sensor: ${sensor?.raw_value ?: 0} °C",
                            fontWeight = FontWeight.SemiBold,
                            fontSize = 27.sp,
                            color = Color.White
                        )
                    }
                }
            }
        }
    }
}

/* ---------------------------------------------------------
   2) MODO AUTOMÁTICO
--------------------------------------------------------- */
@Composable
fun ModoAutomatico() {
    val (actuator, loading, error) = DatosActuator()

    // Estado local sincronizado con Firebase
    var activo by rememberSaveable { mutableStateOf(false) }

    // Cada vez que cambie actuator?.enabled, actualizamos el estado local
    LaunchedEffect(actuator?.enabled) {
        actuator?.enabled?.let { activo = it }
    }

    val colorBoton =
        if (activo) Color(0xFF00C853)   // VERDE
        else Color.Red                  // ROJO

    val textoBoton =
        if (activo) "ACTIVO"
        else "NO ACTIVO"

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(20.dp)
            .background(Color.LightGray),
        verticalArrangement = Arrangement.spacedBy(6.dp)
    ) {
        Box(
            modifier = Modifier
                .height(37.dp)
                .width(250.dp)
                .clip(RoundedCornerShape(
                    topStart = 3.dp,
                    topEnd = 3.dp,
                    bottomEnd = 3.dp
                ))
                .background(Color.DarkGray)
                .padding(end = 18.dp),
            contentAlignment = Alignment.Center    // 👈 CENTRA EL TEXTO
        ) {
            Text(
                text = "Configuración de la ventilación",
                color = Color.White
            )
        }

        Box(
            modifier = Modifier
                .height(35.dp)
                .width(200.dp).offset(y = (-18).dp)
                .clip(RoundedCornerShape(
                    bottomStart = 3.dp,
                    bottomEnd = 3.dp
                ))
                .background(Color.DarkGray)
                .padding(end = 14.dp),
            contentAlignment = Alignment.Center    // 👈 CENTRA EL TEXTO
        ) {
            Text(
                text = "MODO AUTOMÁTICO:",
                color = Color.White
            )
        }

        when {
            loading -> {
                Text(text = "Cargando estado automático...")
            }
            error != null -> {
                Text(text = "Error al leer actuator: $error", color = Color.Red)
            }
            else -> {
                Spacer(modifier = Modifier.height(16.dp))

                Button(
                    onClick = {
                        activo = !activo

                        val valorEnviar = ActuatorControl(
                            enabled = activo,
                            intensity = actuator?.intensity ?: 0,
                            minIntensity = 15,
                            maxIntensity = 255,
                            mode = "automatico"
                        )

                        escribirFirebase("ActuatorControl", valorEnviar)
                    },
                    modifier = Modifier
                        .width(250.dp)
                        .height(55.dp)
                        .padding(start=108.dp),
                    colors = ButtonDefaults.buttonColors(
                        containerColor = colorBoton
                    ),
                    shape = RoundedCornerShape(10.dp)
                ) {
                    Text(textoBoton)
                }
            }
        }
    }
}

/* ---------------------------------------------------------
   3) MODO MANUAL
--------------------------------------------------------- */
@Composable
fun ModoManual() {
    val (actuator, loading, error) = DatosActuator()

    if (loading) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(20.dp)
        ) {
            Text(text = "Cargando modo manual...")
        }
        return
    }

    if (error != null) {
        Column(
            modifier = Modifier
                .fillMaxWidth()
                .padding(20.dp)
        ) {
            Text(text = "Error al leer datos: $error", color = Color.Red)
        }
        return
    }

    // Si llega aquí, podemos usar actuator (aunque igual puede ser null, lo cuidamos)
    val enabledActual = actuator?.enabled ?: false

    // Intensidad inicial sincronizada con Firebase
    var intensidadTexto by rememberSaveable(actuator?.intensity) {
        mutableStateOf(actuator?.intensity?.toString().orEmpty())
    }

    var mensaje by rememberSaveable { mutableStateOf<String?>(null) }

    Column(
        modifier = Modifier
            .fillMaxWidth()
            .padding(20.dp),
        verticalArrangement = Arrangement.spacedBy(16.dp)
    ) {
        Box(
            modifier = Modifier
                .height(37.dp)
                .width(250.dp)
                .clip(RoundedCornerShape(
                    topStart = 3.dp,
                    topEnd = 3.dp,
                    bottomEnd = 3.dp
                ))
                .background(Color.DarkGray)
                .padding(end = 18.dp),
            contentAlignment = Alignment.Center    // 👈 CENTRA EL TEXTO
        ) {
            Text(
                text = "Configuración de la ventilación",
                color = Color.White
            )
        }

        Box(
            modifier = Modifier
                .height(35.dp)
                .width(200.dp).offset(y = (-28).dp)
                .clip(RoundedCornerShape(
                    bottomStart = 3.dp,
                    bottomEnd = 3.dp
                ))
                .background(Color.DarkGray)
                .padding(end = 26.dp),
            contentAlignment = Alignment.Center    // 👈 CENTRA EL TEXTO
        ) {
            Text(
                text = "MODO MANUAL:",
                color = Color.White
            )
        }

        OutlinedTextField(
            value = intensidadTexto,
            onValueChange = { newValue ->
                if (newValue.all { it.isDigit() } || newValue.isEmpty()) {
                    intensidadTexto = newValue
                }
            },
            label = { Text("Intensidad (0 - 255)") },
            placeholder = { Text("Ej: 120") },
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Number),
            modifier = Modifier
                .width(390.dp)
                .align(Alignment.CenterHorizontally)
        )

        if (mensaje != null) {
            Text(
                text = mensaje!!,
                color = Color.Red
            )
        }

        Button(
            onClick = {
                val intensidad = intensidadTexto.toIntOrNull()

                if (intensidad == null || intensidad !in 0..255) {
                    mensaje = "Ingresa un valor numérico entre 0 y 255."
                } else {

                    val valorEnviar = ActuatorControl(
                        enabled = enabledActual,
                        intensity = intensidad,
                        minIntensity = 0,
                        maxIntensity = 255,
                        mode = "manual"
                    )

                    escribirFirebase(
                        field = "ActuatorControl",
                        value = valorEnviar
                    )

                    /*mensaje = if (enabledActual) {
                        "Intensidad guardada y el sistema está ENCENDIDO."
                    } else {
                        "Intensidad guardada, pero el sistema está APAGADO."
                    }*/
                }
            },
            modifier = Modifier
                .width(100.dp)
                .height(48.dp).align(Alignment.CenterHorizontally),
            colors = ButtonDefaults.buttonColors(
                containerColor = Color(0xFF2196F3)
            ),
            shape = RoundedCornerShape(10.dp)
        ) {
            Text(text = "OK")
        }
    }
}
