package com.example.appalimentadormascotas.Models;

import android.content.ContentValues;
import android.content.Context;
import android.database.Cursor;
import android.database.sqlite.SQLiteDatabase;
import android.database.sqlite.SQLiteOpenHelper;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;

import androidx.annotation.Nullable;

import java.io.ByteArrayOutputStream;
import java.util.ArrayList;
import java.util.List;

public class DataBase extends SQLiteOpenHelper {

    public static final String MASCOTAS_TB = "MASCOTAS_TB";
    public static final String COLUMN_NOMBRE = "NOMBRE";
    public static final String COLUMN_EDAD = "EDAD";
    public static final String COLUMN_TIPO = "TIPO";
    public static final String COLUMN_MASCOTA_ID = "MASCOTA_ID";
    public static final String COLUMN_PESO = "PESO";
    public static final String COLUMN_IMG = "IMG";
    private ByteArrayOutputStream objectByteArrayOutputStream;
    private byte[] imageInByte;

    public DataBase(@Nullable Context context) {
        super(context, "mascotas.db", null,1);
    }

    @Override
    public void onCreate(SQLiteDatabase db) {
        String createTableStatement = "CREATE TABLE " + MASCOTAS_TB + " (" + COLUMN_MASCOTA_ID + " INTEGER PRIMARY KEY AUTOINCREMENT," +
                COLUMN_NOMBRE + " TEXT," + COLUMN_EDAD + " INT," + COLUMN_TIPO + " TEXT,"+COLUMN_PESO+ " FLOAT,"+COLUMN_IMG + " BLOB )";
        db.execSQL(createTableStatement);
    }

    @Override
    public void onUpgrade(SQLiteDatabase db, int oldVersion, int newVersion) {

    }

    public boolean addOne(Mascota mascota, Image imagen){
        deleteDB();
        SQLiteDatabase db = this.getWritableDatabase();
        objectByteArrayOutputStream = new ByteArrayOutputStream();
        Bitmap imageToStore = imagen.getImage();
        long insert;
        try{
            imageToStore.compress(Bitmap.CompressFormat.JPEG,50,objectByteArrayOutputStream);
            imageInByte = objectByteArrayOutputStream.toByteArray();
            ContentValues cv = new ContentValues();
            cv.put(COLUMN_NOMBRE,mascota.getNombre());
            cv.put(COLUMN_EDAD,mascota.getEdad());
            cv.put(COLUMN_PESO,mascota.getPeso());
            cv.put(COLUMN_TIPO,mascota.getTipo());
            cv.put(COLUMN_IMG,imageInByte);
            insert = db.insert(MASCOTAS_TB, null, cv);
        }
        catch (Exception e){
            System.out.println(e.getMessage());
            insert = -1;
        }


        if (insert == -1){
            return false;
        }
        else{
            return true;
        }

    }

    public List<Mascota> getMascotas(){
        List<Mascota> returnList = new ArrayList<>();
        String queryString = "SELECT " +COLUMN_NOMBRE+","+COLUMN_EDAD+","+COLUMN_TIPO+","+COLUMN_PESO +" FROM "+MASCOTAS_TB;

        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery(queryString,null);

        if (cursor.moveToFirst()){
            do{
                int mascotaID = cursor.getInt(0);
                String mascotaName = cursor.getString(1);
                int mascotaEdad = cursor.getInt(2);
                String mascotaTipo = cursor.getString(3);
                int mascotaPeso = cursor.getInt(4);

                Mascota mascota = new Mascota(mascotaID,mascotaName,mascotaEdad,mascotaTipo,mascotaPeso);
                returnList.add(mascota);

            }while(cursor.moveToNext());
        }
        cursor.close();
        db.close();
        return returnList;
    }

    public Mascota getOne(){
        Mascota returnMascota = new Mascota();
        String queryString = "SELECT " +COLUMN_MASCOTA_ID+" , "+COLUMN_NOMBRE+" , "+COLUMN_EDAD+" , "+COLUMN_TIPO+" , "+COLUMN_PESO +" FROM "+MASCOTAS_TB;


        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery(queryString,null);

        if (cursor.moveToFirst()){
            do{
                returnMascota.setIdMascota(cursor.getInt(0));
                returnMascota.setNombre(cursor.getString(1));
                returnMascota.setEdad(cursor.getInt(2));
                returnMascota.setTipo(cursor.getString(3));
                returnMascota.setPeso(cursor.getInt(4));
            }while(cursor.moveToNext());
        }
        cursor.close();
        db.close();
        return returnMascota;
    }

    public Image getImage(){
        Image returnImage = new Image();
        String queryString = "SELECT " +COLUMN_IMG+" FROM "+MASCOTAS_TB;


        SQLiteDatabase db = this.getReadableDatabase();
        Cursor cursor = db.rawQuery(queryString,null);

        if (cursor.moveToFirst()){
            do{
                byte[] bytesImage = cursor.getBlob(0);
                Bitmap bm = BitmapFactory.decodeByteArray(bytesImage, 0 ,bytesImage.length);
                returnImage.setImage(bm);
            }while(cursor.moveToNext());
        }
        cursor.close();
        db.close();
        return returnImage;
    }

    public void deleteDB(){
        SQLiteDatabase db = this.getWritableDatabase();
        String deleteString = "DELETE FROM "+MASCOTAS_TB;
        db.execSQL(deleteString);
    }
}

