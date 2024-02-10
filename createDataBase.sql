DROP TABLE IF EXISTS QUIZZ;

CREATE TABLE QUIZZ (
    ID INT,
    QUESTION TEXT,
    OPT1 TEXT,
    OPT2 TEXT,
    OPT3 TEXT,
    OPT4 TEXT,
    OPT_C INT,
    TIME INT
);

INSERT INTO QUIZZ VALUES(1, 'Care este capitala Franței?', 'Berlin', 'Madrid', 'Londra', 'Paris', 4, 30);
INSERT INTO QUIZZ VALUES(2, 'Care este capitala Romaniei?', 'Bucuresti', 'Helsinki', 'Paris', 'Ottawa', 1, 30);
INSERT INTO QUIZZ VALUES(3, 'Care este capitala Norvegiei?', 'New Delhi', 'Oslo', 'Kiev', 'Madrid', 2, 30);
INSERT INTO QUIZZ VALUES(4, 'Care este capitala Spaniei?', 'Stockholm', 'Madrid', 'Berlin', 'Nairobi', 2, 30);
INSERT INTO QUIZZ VALUES(5, 'Care este capitala Portugaliei?', 'Parma', 'Roma', 'Buenos Aires', 'Lisabona', 4, 30);
