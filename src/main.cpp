#include "tree.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>

using namespace std;


int nOfSpam = 100;
int nOfHam = 100;

class Word{
public:
	string name; //the word itself
    int spamMails; //amount of spam mails containing the word
    int hamMails; //amoun of ham mails containing the word
	float pSpam; //the possibility of the word being spam
	float pHam;	//the possibility of the word being non-spam
    int check; //check if the word is already counted in one email
    

    Word() : name(""), spamMails(0), hamMails(0), pSpam(0.0), pHam(0.0), check(0) {}
    Word(string word) : name(word), spamMails(0), hamMails(0), pSpam(0.0), pHam(0.0), check(0) {}
    
    
    void setSpamMails(int spam) {spamMails = spam;}
    void updateSpamMails() {spamMails++;}
    int getSpamMails() {return spamMails ; }
    void setHamMails(int ham) {hamMails = ham;}
    void updateHamMails() {hamMails++; }
    int getHamMails() {return hamMails; }
    void setCheck(int emailNo) {check = emailNo; }
    int getCheck() {return check; }

    void calculateProbabilities(int totalSpam, int totalHam) {
        pSpam = static_cast<float>(spamMails) / totalSpam;
        pHam = static_cast<float>(hamMails) / totalHam;

    }
};

class TrainingMails{
public:
    struct Mail {
        string num; //number of email (1~100)
        string status; // whether it is spam or ham
        string content; //content of the email
    };
    
    vector<Mail> mails;

    map<string, Word> wordList;

    int readEmailFile(string filename){ //return the amount of mails read
        ifstream file(filename);
        if(!file.is_open()) {
            cerr << "Error opening file!" << endl;
            return 0;
        }
        string line;
        int count = 0;
        getline(file, line); // skipping the first line

        while(getline(file, line)) {
            stringstream ss(line);
            string num, status, content;

            getline(ss, num, ',');
            getline(ss, status, ',');
            getline(ss, content, ',');
            count++;
            mails.push_back({num, status, content});
        }
        file.close();
        return count;
    }

    bool checkIfPunctuation(char c) { return ispunct(c); }

    int analyzeWords() {
        int count = 0;
        for (const auto& mail : mails) {
            istringstream stream(mail.content);
            string word;
            count ++;
            while(stream >> word) {
                //removing punctuations
                word.erase(remove_if(word.begin(), word.end(), [this](char c) { return this->checkIfPunctuation(c); }), word.end());
                
                //changing into lowercase
                transform(word.begin(), word.end(), word.begin(), ::tolower);

                //check if the word is already in the list, add new if it already exist, increment the count 
                if (wordList.find(word) == wordList.end()){
                    Word newWord = Word(word);
                    if(mail.status == "spam") newWord.updateSpamMails();
                    else newWord.updateHamMails();
                    newWord.setCheck(count);
                    wordList[word] = newWord;
                } else { //if it is already in the list, check if it already counted in current email, if not, then add
                    if(wordList[word].getCheck() != count){
                        if(mail.status == "spam") wordList[word].updateSpamMails();
                        else wordList[word].updateHamMails();
                    }
                }
            }   
        }
        return count;
    }
    
    void printAllWords() {
        for (const auto& pair : wordList) {
            cout << pair.first << "\t" << endl;
        }
    }

};


int main() {
	TrainingMails trainingMails;
    cout << "Reading file..." << endl;
    trainingMails.readEmailFile("../emails/train/dataset_ham_train100.csv");
    
    cout << "Analyzing words..." << endl;
    trainingMails.analyzeWords();

    cout << "Words analyzed: " << endl;
    trainingMails.printAllWords();
	return 0;
}

