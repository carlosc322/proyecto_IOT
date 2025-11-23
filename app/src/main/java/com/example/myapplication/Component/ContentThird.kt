package com.example.myapplication.Component

// ✔ Jetpack Compose
import androidx.compose.runtime.*
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.componentestest.Componentes.Firebase.SensorData
import com.google.firebase.Firebase
import com.google.firebase.database.database

// ✔ Fecha y hora
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale



@Composable
fun HistorialSensor() {

    // lista donde se guardan los datos del sensor
    var listaDatos by remember { mutableStateOf(listOf<SensorData>()) }
    var cargando by remember { mutableStateOf(false) }

    Column(
        modifier = Modifier
            .fillMaxSize()
            .padding(20.dp)
    ) {
        Text(text = "Los 10 últimos registros")

        Spacer(modifier = Modifier.height(16.dp))

    }
}






fun convertirFecha(segundos: Long?): String {
    if (segundos == null) return "Sin fecha"
    val millis = segundos * 1000L        // En tu base está en segundos
    val fecha = Date(millis)
    val formato = SimpleDateFormat("dd/MM/yyyy HH:mm", Locale.getDefault())
    return formato.format(fecha)
}
