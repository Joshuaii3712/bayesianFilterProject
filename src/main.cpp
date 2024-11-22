#include <iostream>
#include <sstream>
#include <fstream>
#include <map>
#include <vector>
#include <string>
#include <cctype>
#include <algorithm>
#include <cmath>
#include <iomanip>

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
    float getPSpam() { return pSpam; }
    float getPHam() { return pHam; }


    void calculateProbabilities(int totalSpam, int totalHam) {
        if(spamMails == 0) spamMails = 1; //Apply Laplace Smoothing to avoid that p results in 0;
        if(hamMails == 0) hamMails = 1;
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

    map<string, Word>* getMapAddress() { return &wordList; }

    int readEmailFile(string filename){ //return the amount of mails read
        ifstream file(filename);
        if(!file.is_open()) {
            cerr << "Error opening file!" << endl;
            return 0;
        }
        string line, currentText;
        Mail currentMail;
        bool isReadingContent = false;
        int count = 0;
        getline(file, line); // skipping the first line

        while(getline(file, line)) {
            if(isReadingContent) {
                currentText += " " + line;

            if (!line.empty() && line.back() == '"' && line.find("\"\"") == string::npos) {
                isReadingContent = false;
                currentMail.content = currentText.substr(1, currentText.size() - 2); // Strip surrounding quotes
                mails.push_back(currentMail);
                count++;
                currentText.clear();
            }
            continue;
        }


            stringstream ss(line);
            string num, status, content;

            if (!getline(ss, num, ',')) continue;
            if (!getline(ss, status, ',')) continue;
            if (!getline(ss, content)) continue;

        // Parse number
        try {
            currentMail.num = stoi(num);
        } catch (...) {
            throw runtime_error("Invalid number format: " + num);
        }

        currentMail.status = status;

        // Check if the content spans multiple lines
        if (content.front() == '"' && content.back() != '"') {
            isReadingContent = true;
            currentText = content;
            continue;
        }

        // Single-line content
        if (content.front() == '"' && content.back() == '"') {
            currentMail.content = content.substr(1, content.size() - 2); // Strip surrounding quotes
        } else {
            currentMail.content = content;
        }

        mails.push_back(currentMail);
        count++;
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
    
    void calculateProbabilities(int totalSpam, int totalHam) {
        for (auto& pair : wordList) {
            pair.second.calculateProbabilities(totalSpam, totalHam);
        }
    }

    void printAllWords() {
        for (const auto& pair : wordList) {
            cout << pair.first << "|" << pair.second.spamMails << "|" << pair.second.hamMails << "|" << pair.second.pSpam << "|" << pair.second.pHam << endl;
        }
    }

};

class FilterManager {
    public: 
    struct TargetMail {
        string num; //number of target email (101~120)
        string status; // whether it is spam or ham
        string content; // content of the email
        double rOfMail; // the final probability of mails which determines whether the mail is spam or ham
    };

    vector<TargetMail> targetMails;
    
    int readTargetMails(string filename){ //return the amount of mails read
        ifstream file(filename);
        if(!file.is_open()) {
            cerr << "Error opening file!" << endl;
            return 0;
        }
        string line, currentText;
        TargetMail currentMail;
        bool isReadingContent = false;
        int count = 0;
        getline(file, line); // skipping the first line

        while(getline(file, line)) {
            if(isReadingContent) {
                currentText += " " + line;

            if (!line.empty() && line.back() == '"' && line.find("\"\"") == string::npos) {
                isReadingContent = false;
                currentMail.content = currentText.substr(1, currentText.size() - 2); // Strip surrounding quotes
                //cout << currentMail.num << "\t" << currentMail.status << "\n" << currentMail.content << endl;
                targetMails.push_back(currentMail);
                count++;
                currentText.clear();
            }
            continue;
            }

            stringstream ss(line);
            string num, status, content;

            if (!getline(ss, num, ',')) continue;
            if (!getline(ss, status, ',')) continue;
            if (!getline(ss, content)) continue;

        
           currentMail.num = num;
           currentMail.status = status;

        // Check if the content spans multiple lines
        if (content.front() == '"' && content.back() != '"') {
            isReadingContent = true;
            currentText = content;
            continue;
        }

        // Single-line content
        if (content.front() == '"' && content.back() == '"') {
            currentMail.content = content.substr(1, content.size() - 2); // Strip surrounding quotes
        } else {
            currentMail.content = content;
        }
        targetMails.push_back(currentMail);
        count++;
    }

    file.close();
        return count;
    }

    double receiveThreshold() {
        double t;
        printf("Please input the threshold(decimal between 0 to 1) >>");
        cin >> t;
        return t;
    }

    
    bool checkIfPunctuation(char c) { return ispunct(c); }

    int filterMails(map<string, Word>* wordListPtr){
        if (wordListPtr == nullptr) {
            cout << "Map pointer is null." << endl;
            return 0;
        }
        double maxVal, expSpam, expHam;
        int count = 0;
        for (auto& mail : targetMails) {
            long double totalPSpam = 1.0; //get total multipicaltion of pSpam by adding the log of each, and convert the result with exp()
            long double totalPHam = 1.0;
            istringstream stream(mail.content);
            string word;
            count ++;
            while(stream >> word) {
                //removing punctuations
                word.erase(remove_if(word.begin(), word.end(), [this](char c) { return this->checkIfPunctuation(c); }), word.end());
                
                //changing into lowercase
                transform(word.begin(), word.end(), word.begin(), ::tolower);

                //check if the word is in the wordList that we make for filter, if exist, get the pSpam and pHam value and multiply it to the total value;
                if (wordListPtr->find(word) != wordListPtr->end()){
                    totalPSpam *= (*wordListPtr)[word].getPSpam(); 
                    totalPHam *= (*wordListPtr)[word].getPHam();
                    //cout << (*wordListPtr)[word].getPSpam() << "\t" << (*wordListPtr)[word].getPHam() << endl;
                } 
            }   
            //cout << totalPSpam << "\t" << totalPHam << endl;
            //maxVal = max()
            mail.rOfMail = totalPSpam /(totalPSpam + totalPHam);
           // cout << mail.rOfMail << endl;
        }
        return count;
    }

    void printFilterResult() {
        double threshold = receiveThreshold();
        string isSpam, isSuccess;
        int count = 0;
        int success = 0;
        float successRate = 0.0;
        cout << "Num\tStatus\tthreshold\tSpamPercentage\tFilterResult\tSuccess or Fail" << endl;
        for (auto& mail: targetMails) {
            if(mail.rOfMail > threshold ) isSpam = "spam";
            else isSpam = "ham";

            if (mail.status == isSpam) {
                isSuccess = "Success";
                success++;
            } else {
                isSuccess = "Fail";
            }
            count++;
            cout << mail.num << setw(12) <<  mail.status << setw(10) << threshold << setw(20) << mail.rOfMail << setw(15) << isSpam << setw(15) << isSuccess << endl;
            
        }

        successRate = success * 1.0 / count * 1.0;
        cout <<"\nAccuracy for the threshold " << threshold << " : " << successRate << endl;

    }


};



int main() {
	TrainingMails trainingMails;
    FilterManager filterManager;
    int spamNum, hamNum, spamTest, hamTest, readTargetMails;
    cout << "Reading file..." << endl;
    spamNum = trainingMails.readEmailFile("../emails/train/dataset_spam_train100.csv");
    hamNum = trainingMails.readEmailFile("../emails/train/dataset_ham_train100.csv");

    cout <<  spamNum << " spam mails and " << hamNum << " ham mails have been read" << endl;
    
    cout << "Analyzing words..." << endl;
    trainingMails.analyzeWords();

    cout << "Calculating probabilites..." << endl;
    trainingMails.calculateProbabilities(spamNum, hamNum);

   // cout << "Words analyzed: " << endl;
    //trainingMails.printAllWords();

    spamTest = filterManager.readTargetMails("../emails/test/dataset_spam_test20.csv");
    hamTest = filterManager.readTargetMails("../emails/test/dataset_ham_test20.csv");

    cout <<  spamTest << " spam target mails and " << hamTest << " ham target mails have been read" << endl;

    readTargetMails = filterManager.filterMails(trainingMails.getMapAddress());
    cout << readTargetMails << " mails have been read." << endl;

    int quit;
    while(1){
        filterManager.printFilterResult();
        printf("\nDo you want to end(0) or continue with new threshold(1)?");
        cin >> quit;
        if(quit == 0) break;
    }
    

    cout << exp(log(0.6) + log(0.1)) << endl;
    cout << "Good bye!" << endl;

	return 0;
}