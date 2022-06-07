package com.example.appalimentadormascotas.Models;

import android.content.Context;
import android.content.Intent;
import android.provider.Settings;

import com.example.appalimentadormascotas.Activities.mainActivity;
import com.example.appalimentadormascotas.Contracts.Contract;

import java.util.ArrayList;
import java.util.List;

public class Model implements  Contract.Model{
    private List<Mascota> arrayList = new ArrayList<Mascota>();
    public Mascota mascota;
    public DataBase db;




  /*  @Override
    public void getNextName(final OnEventListener listener){
        listener.onEvent(getRandomMascota(),1);
    }
*/
    private String saveMascotasDB(Mascota mascota, Image imagen, Contract.View mainView){
        db = new DataBase((Context) mainView);
        db.deleteDB();
        String resultado;
        try {
            db.addOne(mascota,imagen);
            resultado = "Insertado";
        }
        catch (Exception e){
            resultado = "Error al realizar la inserción";
        }
        return resultado;
    }


    @Override
    public void saveMascota(Mascota mascota, Image imagen, Contract.View setupView, OnEventListener listener) {
        listener.onEvent(saveMascotasDB(mascota, imagen, setupView),Contract.SAVE_MASCOTA_EVENT);
    }








}