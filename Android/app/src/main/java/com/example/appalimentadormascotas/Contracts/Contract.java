package com.example.appalimentadormascotas.Contracts;

import android.os.Bundle;

import com.example.appalimentadormascotas.Models.Image;
import com.example.appalimentadormascotas.Models.Mascota;
import com.example.appalimentadormascotas.Presenters.Presenter;

public interface Contract {
    public static final int SAVE_MASCOTA_EVENT = 1;
    public static final int SAVE_IMG_EVENT = 2;
    public static final int TO_APP_EVENT = 3;

        interface View{
            void onCreate(Bundle savedInstanceState);
            void onDestroy();
            void setString(String string);
        }
        interface Model{

            interface OnEventListener {
                void onEvent(String string, int evento);
            }
          //  void getNextName(Contract.Model.OnEventListener listener);
            void saveMascota(Mascota mascota, Image image, View setupView, OnEventListener listener);

        }
        interface Presenter{
            void onSaveButtonClick(Mascota mascota,Image image);
            void onImgButtonClick();
            void onToAppButtonClick();
            void onLightSensorChanged(Float sensorValue);
            void onDestroy();
            void onCreate();
        }


}
