package com.example.appalimentadormascotas.Activities;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.SharedPreferences;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.Message;
import android.provider.Settings;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.example.appalimentadormascotas.Contracts.Contract;
import com.example.appalimentadormascotas.Models.BluetoothThread;
import com.example.appalimentadormascotas.Models.DataBase;
import com.example.appalimentadormascotas.Models.Image;
import com.example.appalimentadormascotas.Models.Mascota;
import com.example.appalimentadormascotas.Presenters.Presenter;
import com.example.appalimentadormascotas.R;

public class mainActivity extends AppCompatActivity implements  Contract.View {

    //Componentes Visuales
    private TextView nombreMascota;
    private TextView edadMascota;
    private TextView tipoMascota;
    private TextView pesoMascota;
    public TextView platoMascota;
    private TextView dispenserMascota;
    private ImageView imagenMascota;
    private TextView lightSensorTV;
    private Button bluetoothBTN;
    private Button verSensoresBTN;
    private Button apagarLEDBTN;
    private Button perfilBTN;

    //Componentes Necesarios para manejo del sensor de luz
    private SensorManager sensorManager;
    private Sensor sensorLight;
    private float SENSOR_LIGHT_MAX_VALUE = 40;
    private float SENSOR_LIGHT_CONST_VALUE = 255;
    SensorEventListener sensorEventListenerLight;

    //Componentes Necesarios para manejo de Bluetooth
    BluetoothThread bluetoothThread = null;
    Handler writeHandler;
    String address = null;


    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main2);

        //Inicializó los componentes Visuales
        inicializarElementos();

        //Pido permiso al usuario para realizar cambios sobre el sistema
        //En nuestro caso el brillo de la pantalla
        askPermissionChangeBrightness();

        //Configuro el Sensor de Luz
        configureLightSensor();

        //Añado el listener para escuchar a cambios en la luminosidad del ambiente.
        sensorEventListenerLight = new SensorEventListener() {
            //Si el sensor se modifica, modifico el brillo de la pantalla.
            @Override
            public void onSensorChanged(SensorEvent event) {
                float sensorValue = event.values[0];
                int newValue = (int) (sensorValue * SENSOR_LIGHT_CONST_VALUE / SENSOR_LIGHT_MAX_VALUE);
                Settings.System.putInt(mainActivity.this.getContentResolver(), Settings.System.SCREEN_BRIGHTNESS, newValue);
            }
            @Override
            public void onAccuracyChanged(Sensor sensor, int accuracy) {
            }
        };

        //Registro el listener para que comience a escuchar.
        sensorManager.registerListener(sensorEventListenerLight,sensorLight,SensorManager.SENSOR_DELAY_NORMAL);

        //Acción sobre el botón de bluetooth, si es precionado el Intent nos lleva
        //a la Activity Device List
        bluetoothBTN.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(mainActivity.this, DeviceList.class);
                startActivity(intent);
            }
        });
        //Acción sobre el botón de LED OFF, si es presionado escribo sobre el writeHandler
        //Enviandole el mensaje al bluetooth LED_OFF el cual apagará el LED siempre y cuando
        //se encuentre en estado DISPENSER_VACIO
        apagarLEDBTN.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick (View v) {

                Message msg = Message.obtain();
                msg.obj = "LED_OFF";
                writeHandler.sendMessage(msg);

            }
        });

        //Acción sobre el botón de VER SENSORES, si es presionado escribo sobre el writeHandler
        //Enviandole el mensaje al bluetooth GET_SENSOR_VALUE. El arduino entonces responderá
        //envíando un String con los valores de los sensores que serán leídos por el handler de lectura
        verSensoresBTN.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick (View v) {
                Message msg = Message.obtain();
                msg.obj = "GET_SENSOR_VALUE";
                writeHandler.sendMessage(msg);
            }
        });

        //Acción sobre el botón de perfil que nos permite cambiar los datos de perfil en caso de
        //querer cambiarlos.
        perfilBTN.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Intent intent = new Intent(mainActivity.this, setUpActivity.class);
                startActivity(intent);
            }
        });

        //Intent que recibe el address del dispositivo bluetooth conectado y instancia el
        //bluetoothThread.
        Intent newint = getIntent();
        address = newint.getStringExtra(DeviceList.EXTRA_ADDRESS);
        if(address != null){
            instanceBluetoothThread();
            // Get the handler that is used to send messages
            writeHandler = bluetoothThread.getWriteHandler();
            // Run the thread
            bluetoothThread.start();

        }


    }




    @Override
    public void onResume() {
        super.onResume();
        sensorManager.registerListener(sensorEventListenerLight,sensorLight,SensorManager.SENSOR_DELAY_NORMAL);

        }

    @Override
    public void onDestroy() {
        super.onDestroy();
    }

    @Override
    public void onStop() {
        super.onStop();
        sensorManager.unregisterListener(sensorEventListenerLight);
    }

    @Override
    public void setString(String string) {

    }



    private void inicializarElementos(){
        this.nombreMascota = findViewById(R.id.mascotaNombreTB);
        this.edadMascota = findViewById(R.id.mascotaEdadTB);
        this.tipoMascota = findViewById(R.id.mascotaTipoTB);
        this.pesoMascota =  findViewById(R.id.mascotaPesoTB);
        this.platoMascota=  findViewById(R.id.sensorPlatoTV);
        this.dispenserMascota =  findViewById(R.id.sensorDispenserTV);

        this.imagenMascota = findViewById(R.id.mascotaIMG);
        this.lightSensorTV = findViewById(R.id.lightTB);
        this.bluetoothBTN = findViewById(R.id.bluetoothBTN);
        this.verSensoresBTN = findViewById(R.id.verSensoresBTN);
        this.apagarLEDBTN = findViewById(R.id.apagarLedBTN);
        this.perfilBTN = findViewById(R.id.perfilBTN);


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
    private void instanceBluetoothThread() {
        bluetoothThread = new BluetoothThread(address,new Handler(){
            @Override
            public void handleMessage(Message message) {

                String s = (String) message.obj;

                // Do something with the message
                if (s.equals("CONNECTED")) {
                    Toast.makeText(mainActivity.this, "CONNECTED", Toast.LENGTH_SHORT).show();
                } else if (s.equals("DISCONNECTED")) {
                    Toast.makeText(mainActivity.this, "DISCONNECTED", Toast.LENGTH_SHORT).show();

                } else if (s.equals("CONNECTION FAILED")) {
                    Toast.makeText(mainActivity.this, "CONNECTION FAILED", Toast.LENGTH_SHORT).show();

                } else if (s.split(",")[0].equals("SENSOR_VALUE")){
                    Toast.makeText(mainActivity.this, (String)s, Toast.LENGTH_SHORT).show();
                    dispenserMascota.setText("Dispenser:"+(String)s.split(",")[1]);
                    platoMascota.setText("Plato"+(String)s.split(",")[2]);
                }
            }
        });
    }


}