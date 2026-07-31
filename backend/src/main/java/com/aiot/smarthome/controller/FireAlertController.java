package com.aiot.smarthome.controller;


import com.aiot.smarthome.dto.FireAlertResponse;
import com.aiot.smarthome.model.FireAlert;
import com.aiot.smarthome.service.FireAlertService;


import org.springframework.http.ResponseEntity;
import org.springframework.web.bind.annotation.*;
import org.springframework.web.multipart.MultipartFile;


import java.io.IOException;
import java.nio.file.*;
import java.util.List;



@RestController
@RequestMapping("/api/fire-alert")
@CrossOrigin
public class FireAlertController {


    private final FireAlertService service;


    private final Path uploadDir =
            Paths.get("uploads");


    public FireAlertController(
            FireAlertService service
    ){
        this.service = service;
    }



    @PostMapping
    public ResponseEntity<?> uploadFire(

            @RequestParam("image")
            MultipartFile image,


            @RequestParam("deviceId")
            String deviceId,


            @RequestParam("confidence")
            Double confidence


    ) throws IOException {



        Files.createDirectories(uploadDir);



        String filename =
                System.currentTimeMillis()
                +
                ".jpg";



        Path path =
                uploadDir.resolve(filename);



        Files.copy(
                image.getInputStream(),
                path,
                StandardCopyOption.REPLACE_EXISTING
        );



        FireAlert alert =
                service.createAlert(

                        deviceId,

                        path.toString(),

                        confidence

                );



        return ResponseEntity.ok(alert);

    }




    @GetMapping
    public List<FireAlert> getAll(){

        return service.getAlerts();

    }



    @GetMapping("/latest")
    public ResponseEntity<?> latest(){

        return ResponseEntity.ok(
                service.getLatest()
        );

    }

}