package com.example.appalimentadormascotas.Activities;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.appcompat.app.AppCompatActivity;
import androidx.appcompat.app.AppCompatDelegate;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.graphics.drawable.BitmapDrawable;
import android.hardware.Sensor;
import android.hardware.SensorEvent;
import android.hardware.SensorEventListener;
import android.hardware.SensorManager;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.ImageView;
import android.widget.TextView;
import android.widget.Toast;

import com.example.appalimentadormascotas.Contracts.Contract;
import com.example.appalimentadormascotas.Models.Image;
import com.example.appalimentadormascotas.Models.Mascota;
import com.example.appalimentadormascotas.Models.Model;
import com.example.appalimentadormascotas.Presenters.Presenter;
import com.example.appalimentadormascotas.R;

import java.io.FileNotFoundException;
import java.io.InputStream;
import java.util.Objects;

public class setUpActivity extends AppCompatActivity implements Contract.View {

    private TextView nombreTV;
    private TextView edadTV;
    private TextView tipoTV;
    private TextView pesoTV;
    private ImageView fotoMascota;
    private Button imgButton;
    private Button saveButton;
    private Button toAppButton;
    private Mascota mascota;
    public Image imagen;


    private static final int FLAG_INSERTADO = 1;
    private static final int PICK_IMAGE_REQUEST = 100;



    private int flagInsertado = 0;

    Contract.Presenter presenter;

    @Override
    public void onCreate(Bundle savedInstanceState) {

        super.onCreate(savedInstanceState);

        //Linkeo la vista con su correspondiente Layout
        setContentView(R.layout.activity_main);

        //Inicializo los componentes visuales.
        initialize();

        //Instancio el Presentador
        presenter = new Presenter(this, new Model());
        presenter.onCreate();

        /*Función sobre el botón Save
           Guarda la información cargada en la base de datos.
           Para ser guardada deben estar todos los campos ingresables cargados
         */
        saveButton.setOnClickListener(new View.OnClickListener(){
            @Override
            public void onClick(View v) {
                mascota = extractMascotaFromView();
                if(flagInsertado != FLAG_INSERTADO && isMascotaDataReady()){
                    presenter.onSaveButtonClick(mascota,imagen);
                }
               else{
                    System.out.println("La mascota ya ha sido insertada");
                }

            }
        });

        /* Función sobre el botón Image. Llama a la función Choose Image
           la cual se encargará del manejo del Intent para pickear la foto de la galeria
         */
        imgButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                chooseImage(v);
            }
        });
        /*
            Una vez guardados los datos del perfil, a partir del botón TO_APP
            nos podremos ir a la activity principal de la aplicación.
         */
        toAppButton.setOnClickListener(new View.OnClickListener(){

            @Override
            public void onClick(View v) {
                openMainActivity();
            }
        });


    }


    @Override
    public void onDestroy() {
        super.onDestroy();
        presenter.onDestroy();
    }

    /*
       Función que devuelve el presentador una vez se insertó un perfil de mascota
     */
    @Override
    public void setString(String string) {
        Toast.makeText(this, "strng", Toast.LENGTH_SHORT).show();
        flagInsertado = 1;
    }

    // Funcion con el Intent que nos lleva  a la activity principal
    private void openMainActivity(){
        Intent intent = new Intent(this, mainActivity.class);
        startActivity(intent);
    }

    // La función utiliza un Intent de tipo pick para seleccionar una imagen de la galería.
    // el retorno es resuelto con un registerForActivityResult.
    public void chooseImage(View objectView){
        try{
            Intent photoPickerIntent = new Intent(Intent.ACTION_PICK);
            photoPickerIntent.setType("image/*");
            activityResultLauncher.launch(photoPickerIntent);

        }
        catch (Exception e){
            Toast.makeText(this, e.getMessage(), Toast.LENGTH_SHORT).show();
        }
    }

    //Función que inicializa los componentes visuales.
    private void initialize(){
        this.nombreTV = findViewById(R.id.nombreMascotaTB);
        this.edadTV = findViewById(R.id.edadMascotaTB);
        this.tipoTV = findViewById(R.id.tipoMascotaTB);
        this.pesoTV =  findViewById(R.id.pesoMascotaTB);
        this.fotoMascota = findViewById(R.id.mascotaIMG);
        this.imgButton =  findViewById(R.id.imgButton);
        this.saveButton =  findViewById(R.id.saveBTN);
        this.toAppButton =  findViewById(R.id.toAppBTN);
        this.imagen = new Image();

    }
    private boolean isMascotaDataReady(){
        if(this.nombreTV != null && this.edadTV != null && this.tipoTV != null
            && this.pesoTV!= null && this.fotoMascota != null) {
            return true;
        }

        return false;
    }

    private Mascota extractMascotaFromView() {
        Mascota mascotaMT;
        try {
            mascotaMT = new Mascota(-1,nombreTV.getText().toString()
                    ,Integer.parseInt(edadTV.getText().toString())
                    ,tipoTV.getText().toString()
                    ,Integer.parseInt(pesoTV.getText().toString())
            );

        }
        catch(Exception e){
            System.out.println("Error al crear la Mascota:"+e);
            mascotaMT = new Mascota(-1,"Error",0,"Error",0);
        }
        return mascotaMT;
    }

    ActivityResultLauncher<Intent> activityResultLauncher = registerForActivityResult(
            new ActivityResultContracts.StartActivityForResult(),
            result -> {
                if (result.getResultCode() == Activity.RESULT_OK) {
                    // There are no request codes
                    // doSomeOperations();
                    Intent data = result.getData();
                    Uri selectedImage = Objects.requireNonNull(data).getData();
                    InputStream imageStream = null;
                    try {
                        imageStream = getContentResolver().openInputStream(selectedImage);
                    } catch (FileNotFoundException e) {
                        e.printStackTrace();
                    }

                    fotoMascota.setImageURI(selectedImage);// To display selected image in image view
                    imagen.setImage(((BitmapDrawable) fotoMascota.getDrawable()).getBitmap());

                }
            });


}