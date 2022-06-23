package com.example.appalimentadormascotas.Models;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.os.Handler;
import android.os.Message;
import android.util.Log;

import java.io.InputStream;
import java.io.OutputStream;
import java.util.UUID;

public class BluetoothThread extends Thread{
    // Etiqueta
    private static final String TAG = "BluetoothThread";

    // Delimitador para señalar el fin del mensaje
    private static final char PARSE_DELIMITER = '|';
    private static final char  DELIMITER = '\n';



    // UUID que específica el protocolo de comunicación bluetooth
    private static final UUID uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    // MAC address del dispositivo Bluetooth
    private final String address;

    // Socket de la comunicación Bluetooth activa
    private BluetoothSocket socket;

    // Streams de lectura y escritura
    private OutputStream outStream;
    private InputStream inStream;

    // Handlers de lectura y escritura
    private final Handler readHandler;
    private final Handler writeHandler;

    // Buffer utilizado para parsear los mensajes
    private String rx_buffer = "";

    /**
     * El constructor, toma la dirección MAC del dispositivo Bluetooth
     * y el handler para mensajes recibidos

     */
    public BluetoothThread(String address, Handler handler) {

        this.address = address.toUpperCase();
        this.readHandler = handler;

        writeHandler = new Handler() {
            @Override
            public void handleMessage(Message message) {
                write((String) message.obj);
            }
        };
    }

    /**
     * Retorna un handler de escritura para esta conexión.
     * Los mensajes recibidos por este handler serán escritos en el Socket Bluetooth
     */
    public Handler getWriteHandler() {
        return writeHandler;
    }

    /**
     * Conección al Sockt bluetooth, tira una excepción si falla.
     */
    private void connect() throws Exception {

        Log.i(TAG, "Intentando conexión a " + address + "...");

        // Obtiene el adaptador bluetooth
        BluetoothAdapter adapter = BluetoothAdapter.getDefaultAdapter();
        if ((adapter == null) || (!adapter.isEnabled())){
            throw new Exception("El adaptador Bluetooth no está habilitado!");
        }

        // Busca el dispositivo remoto
        BluetoothDevice remoteDevice = adapter.getRemoteDevice(address);

        // Crea el socket utilizando el protocolo señalado
        socket = remoteDevice.createRfcommSocketToServiceRecord(uuid);

        // Asegura que el adaptador no este en discovery mode
        adapter.cancelDiscovery();

        // Finalmente se conecta al socket
        socket.connect();


        // Obtiene el inpunt y output stream del Socket
        outStream = socket.getOutputStream();
        inStream = socket.getInputStream();

        Log.i(TAG, "Conectado satisfactoriamente a  " + address + ".");
    }

    /**
     * Desconexión de los stream y el socket
     */
    private void disconnect() {

        if (inStream != null) {
            try {inStream.close();} catch (Exception e) { e.printStackTrace(); }
        }

        if (outStream != null) {
            try {outStream.close();} catch (Exception e) { e.printStackTrace(); }
        }

        if (socket != null) {
            try {socket.close();} catch (Exception e) { e.printStackTrace(); }
        }
    }

    /**
     * Retorna la data leída por el socket o un String vacio.
     */
    private String read() {

        String s = "";

        try {
            // Chequeo si hay algo en el inputstream
            if (inStream.available() > 0) {

                // Lo pego en el inBuffer
                byte[] inBuffer = new byte[1024];
                int bytesRead = inStream.read(inBuffer);

                // Lo convierto a String
                s = new String(inBuffer, "ASCII");
                s = s.substring(0, bytesRead);
            }

        } catch (Exception e) {
            Log.e(TAG, "Lectura fallida!", e);
        }

        return s;
    }

    /**
     * Escribir en el socket
     */
    private void write(String s) {

        try {
            // Añado el delimitador
            s += DELIMITER;

            // Convierto a bytes y envío
            outStream.write(s.getBytes());
            Log.i(TAG, "[SENT] " + s);

        } catch (Exception e) {
            Log.e(TAG, "Escritura fallo!", e);
        }
    }

    /**
     * pasa el mensaje al read handler.
     */
    private void sendToReadHandler(String s) {
        System.out.println("Envíado al readHandler "+s);
        Message msg = Message.obtain();
        msg.obj = s;
        readHandler.sendMessage(msg);
        Log.i(TAG, "[RECV] " + s);
    }

    /**
     * Envía los mensajes completos del rx_buffer al read handler
     */
    private void parseMessages() {

        // Encuentra el indice del delimitador del buffer
        int inx = rx_buffer.indexOf(PARSE_DELIMITER);

        // Si no hay nada salgo
        if (inx == -1)
            return;

        // Obtengo el mensaje completo
        String s = rx_buffer.substring(0, inx);
        System.out.println("rx_buffer "+s);
        // Remuevo el mensaje del buffer
        rx_buffer = rx_buffer.substring(inx + 1);

        // Manda el mensaje al handler
        sendToReadHandler(s);

        // Busco mas mensajes completos.
        parseMessages();
    }

    /**
     * Arranca cuando se realiza thread.start
     */
    public void run() {

        // Intenta conectar y realiza exit si falla
        try {
            connect();
            sendToReadHandler("CONNECTED");
        } catch (Exception e) {
            Log.e(TAG, "Failed to connect!", e);
            sendToReadHandler("CONNECTION FAILED");
            disconnect();
            return;
        }

        // Loopea continuamente leyendo datos hasya thread.interrupt()
        while (!this.isInterrupted()) {

            // Me fijo si no se callo la conexión
            if ((inStream == null) || (outStream == null)) {
                Log.e(TAG, "Se perdió la conexión!");
                break;
            }

            // Leo datos y los añado al buffer.
            String s = read();
            if (s.length() > 0){
                rx_buffer += s;
                System.out.println("Leyo mensaje"+s);
            }

            // Busco mensajes completos.
            parseMessages();
        }

        // Si el hilo es interrumpido me desconecto.
        disconnect();
        sendToReadHandler("DISCONNECTED");
    }
}
