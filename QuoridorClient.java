import java.io.*;
import java.net.*;
import java.util.Scanner;

public class QuoridorClient {
    private Socket socket;
    private PrintWriter out;
    private BufferedReader in;
    private boolean isRunning;

    public QuoridorClient(String host, int port) throws IOException {
        socket = new Socket(host, port);
        socket.setTcpNoDelay(true);
        out = new PrintWriter(socket.getOutputStream(), true);
        in = new BufferedReader(new InputStreamReader(socket.getInputStream()));
        isRunning = true;
    }

    public void start() {
        // Start a thread to handle server messages
        Thread receiveThread = new Thread(() -> {
            try {
                String message;
                while (isRunning && (message = in.readLine()) != null) {
                    handleServerMessage(message);
                }
            } catch (IOException e) {
                System.err.println("Error receiving message: " + e.getMessage());
                isRunning = false;
            }
        });
        receiveThread.start();

        // Handle user input
        Scanner scanner = new Scanner(System.in);
        while (isRunning) {
            String input = scanner.nextLine();
            handleUserInput(input);
        }
        scanner.close();
    }

    private void handleServerMessage(String message) {
        System.out.println("Received message: " + message);
    }
    /*
    private String extractValue(String jsonString, String key) {
        int startIndex = jsonString.indexOf("\"" + key + "\":\"") + key.length() + 4;
        int endIndex = jsonString.indexOf("\"", startIndex);
        return jsonString.substring(startIndex, endIndex);
    }
    */

    private void handleUserInput(String input) {
        if (input.equals("quit")) {
            isRunning = false;
            close();
            return;
        }
        if (input.equals("move")) {
            out.println("{\"type\":\"move\",\"data\":{\"is_horizontal\":false,\"position\":[1,1]}}");
            return;
        }

        // Create a move message
        String moveMessage = String.format("{\"type\":\"name_response\",\"data\":{\"name\":\"%s\"}}", input);
        out.println(moveMessage);
    }

    private void close() {
        try {
            isRunning = false;
            if (out != null) out.close();
            if (in != null) in.close();
            if (socket != null) socket.close();
        } catch (IOException e) {
            System.err.println("Error closing connection: " + e.getMessage());
        }
    }

    public static void main(String[] args) {
        try {
            QuoridorClient client = new QuoridorClient("localhost", 5002);
            System.out.println("START");
            client.start();
        } catch (IOException e) {
            System.err.println("Failed to connect to server: " + e.getMessage());
        }
    }
} 