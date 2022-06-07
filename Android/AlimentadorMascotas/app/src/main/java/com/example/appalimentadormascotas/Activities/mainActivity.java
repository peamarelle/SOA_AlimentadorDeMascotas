package com.example.appalimentadormascotas.Activities;

import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;

import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.widget.ImageView;
import android.widget.TextView;

import com.example.appalimentadormascotas.Contracts.Contract;
import com.example.appalimentadormascotas.Models.DataBase;
import com.example.appalimentadormascotas.Models.Image;
import com.example.appalimentadormascotas.Models.Mascota;
import com.example.appalimentadormascotas.Presenters.Presenter;
import com.example.appalimentadormascotas.R;

import java.util.List;

public class mainActivity extends AppCompatActivity implements  Contract.View {
    private TextView nombreMascota;
    private TextView edadMascota;
    private TextView tipoMascota;
    private TextView pesoMascota;
    private ImageView imagenMascota;
    private TextView lightSensorTV;

    private SensorManager sensorManager;
    private Sensor sensorLight;
    private float sensorLightMaxValue = 40;


    SharedPreferences sharedPreferences;


    Presenter presenter;

    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);

        inicializarElementos();
        askPermissionChangeBrightness();
        configureLightSensor();

        SensorEventListener sensorEventListenerLight = new SensorEventListener() {
            @Override
            public void onSensorChanged(SensorEvent event) {
                float sensorValue = event.values[0];
                int newValue = (int) (sensorValue * 255 / sensorLightMaxValue);
                lightSensorTV.setText(Integer.toString(newValue));
                Settings.System.putInt(mainActivity.this.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS, newValue);
            }

            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {

            }
        };

        sensorManager.registerListener(sensorEventListenerLight,sensorLight,SensorManager.SENSOR_DELAY_NORMAL);


    }





    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void setString(String string) {

    }



    private void inicializarElementos(){
        this.nombreMascota = findViewById(R.id.mascotaNombreTB);
        this.edadMascota = findViewById(R.id.mascotaEdadTB);
        this.tipoMascota = findViewById(R.id.mascotaTipoTB);
        this.pesoMascota =  findViewById(R.id.mascotaPesoTB);
        this.imagenMascota = findViewById(R.id.mascotaIMG);
        this.lightSensorTV = findViewById(R.id.lightTB);

        DataBase db = new DataBase(mainActivity.this);
        Mascota mascota  = db.getOne();
        Image image = db.getImage();
        System.out.println(mascota.toString());

        this.nombreMascota.setText( "Nombre: "+mascota.getNombre());
        this.tipoMascota.setText( "Tipo: "+mascota.getTipo());
        this.edadMascota.setText( "Edad :"+Integer.toString(mascota.getEdad()));
        this.pesoMascota.setText(  "Peso :"+ Integer.toString(mascota.getPeso()));
        imagenMascota.setImageBitmap(image.getImage());


    }
    private void askPermissionChangeBrightness() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            if (!Settings.System.canWrite(this)) {
                Intent intent = new Intent(Settings.ACTION_MANAGE_WRITE_SETTINGS);
                intent.setData(Uri.parse("package:" + getPackageName()));
                startActivity(intent);
            }
        }
    }
    private void configureLightSensor() {
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.M) {
            sensorManager = (SensorManager)getSystemService(Context.SENSOR_SERVICE);
        }
        else{
            System.out.println("El dispositivo no es compatible con el sensor de Luminosidad");
        }

        sensorLight = sensorManager.getDefaultSensor(Sensor.TYPE_LIGHT);
    }
}