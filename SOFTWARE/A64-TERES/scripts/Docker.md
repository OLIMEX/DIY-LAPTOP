#Using docker to set up build enviorement

## 1.Installing Docker
Ex Ubuntu:
https://docs.docker.com/v17.09/engine/installation/linux/docker-ce/ubuntu/#install-docker-ce

for any other OS folow instruction on docker.com

### 2.Building image

```bash
 cd DIY-LAPTOP/SOFTWARE/A64-TERES/
 docker build  - < Dockerfile
```

  if build is successful,you can list you image with
```bash
docker image list
```
#### 3.Running image
Run :
```bash	
	docker build -q - < Dockerfile	
```
you will recieve as ouptur docker image id, to run it : 
```bash
	docker run -it --rm -v "$(pwd):$(pwd)" -w "$(pwd)" id_from_previous_command
```

Now you can follow instructions in README, just skip  toolchain installation step

