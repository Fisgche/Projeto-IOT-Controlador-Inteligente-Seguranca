import paho.mqtt.client as mqtt

# importe o conector do Python com o MySQL: instalar novamente neste env
import mysql.connector

# agora é necessário criar um objeto de conexão: encontra o MySQL e informa as credenciais para se conectar ao banco
# instalar novamente o concetor: pip install mysql-connector-python
con = mysql.connector.connect(host='localhost', database='banco_IOT', user='root', password='260102')

# verifique se a conexão ao BD foi realizada com sucesso
if con.is_connected():
    db_info = con.get_server_info()
    print("Conentado com sucesso ao Servidor ", db_info)

    # a partir de agora pode-se executar comandos SQL: para tanto é necessário criar um objeto tipo cursor
    # o cursor permite acesso aos elementos do BD
    cursor = con.cursor()

    # agora você pode executar o seu comando SQL. Por exemplo o comando "select database();" mostra o BD atual selecionado
    cursor.execute("select database();")
    # crio uma variável qualquer para receber o retorno do comando de execução
    linha = cursor.fetchone()
    print("Conectado ao DB", linha)

    # createTable é usada no comando SQL para  criar a tabela dadosIoT, que só tem um registro chamado "mensagem"
    createTable = """
                CREATE TABLE Morador (id INT AUTO_INCREMENT, 
                                       nome TEXT(512),
                                       Scodigo1 INT,
                                       Scodigo2 INT, 
                                       PRIMARY KEY (id));
            """

    # este par try/except verifica se a tabeja  já está criada. Se a tabela não existe, cai no try e é criada
    # se existe, cai no except e só mostra a mensagem que  a tabela existe
    try:
        cursor.execute(createTable)
    except:
        print("Tabela Morador já existe.")
        pass
    createTable = """
                    CREATE TABLE Buzzer (id INT AUTO_INCREMENT,
                                        codigo2 INT, 
                                        valor TEXT(512), 
                                        data DATE,
                                        hora TIME,
                                        PRIMARY KEY (id));
                    """
    try:
        cursor.execute(createTable)
    except:
        print("Tabela Buzzer já existe.")
        pass



def print_hi(name):
    # mensagem inicial
    print(f'Hi, {name}')  # Press Ctrl+F8 to toggle the breakpoint.

# esta função é a função callback informando que o cliente se conectou ao servidor
def on_connect(client, userdata, flags, rc):
    print("Connectado com codigo "+str(rc))

    # assim que conecta, assina um tópico. Se a conexão for perdida, assim que
    # que reconectar, as assinaturas serão renovadas
    client.subscribe("topico_sensor_temperatura")


# esta função é a função callback que é chamada quando uma publicação é recebida do servidor
def on_message(client, userdata, msg):
    print("Mensagem recebida no tópico: " + msg.topic)
    print("Mensagem: "+ str(msg.payload.decode()) + "º")

    # ao receber um dado, insere como um registro da tabela dadosIoT
    cursor = con.cursor()
    cursor.execute("INSERT INTO dadosIoT (mensagem) VALUES ('{}')".format(str(msg.payload.decode())))
    con.commit()

    cursor.execute("SELECT * FROM dadosIoT")
    myresult = cursor.fetchall()
    print(myresult)

    if str(msg.payload.decode().strip()) == "termina":
        print("Recebeu comando termina.")
        if con.is_connected():
            cursor.close()
            con.close()
            print("Fim da conexão com o Banco dadosIoT")

    if str(msg.payload.decode().strip())  == "delete":
        cursor.execute("TRUNCATE TABLE dadosIoT")


if __name__ == '__main__':
    print_hi('Olá Turma.')

    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message

    client.connect("test.mosquitto.org", 1883, 60)
    #client.connect("broker.hivemq.com", 1883, 60) # broker alternativo

    # a função abaixo manipula trafego de rede, trata callbacks e manipula reconexões.
    client.loop_forever()

