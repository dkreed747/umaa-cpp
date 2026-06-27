#  Copyright 2025 Pennsylvania State University
#
#  Applied Research Laboratory
#  Pennsylvania State University
#  P.O. Box 30
#  State College, PA 16804-0030
#
#  DISTRIBUTION STATEMENT A. Approved for public release.
#  Distribution is unlimited.
#  This software was developed by the Department of the Navy,
#  NAVSEA Unmanned and Small Combatants. It is provided under the terms of
#  use found in the LICENSE file at the source code root directory.
#

import gitlab
import sys
import argparse


"""
Script to update parent projects containing umaa-sdk-common as a Git submodule.
"""

def getLatestCommit(project, branch_name) -> tuple[str, str]:
    """
    Returns the most recent commit for a given project and branch.
    :param project: The project to get the most recent commit
    :return: Information about the latest commit
    """

    return project.commits.list(ref_name=branch_name, get_all=False)[0]
    
def createIssue(project, commit_id, commit_title) -> int:
    issue_args = {'title': f'AUTO: Update umaa-sdk-common submodule {commit_id[:8]}',
                  'description': f"""\
**THIS ISSUE WAS AUTO-GENERATED**

Update umaa-sdk-common to commit sha: {commit_id}

Branch name: \"{commit_title}\""""}

    issue = project.issues.create(issue_args)
    issue.labels = ['AUTO-GENERATED']
    issue.save()
    return issue.iid

def closeIssue(project, iid) -> None:
    issue = project.issues.get(iid)
    issue.state_event = 'close'
    issue.save()
    
def createBranch(project, branch_name) -> None:
    """
    Creates a branch from main on a given project.
    :param project: The project to branch
    :param branch_name: The name of the new branch
    """
    branch_args = {'branch': branch_name,
                   'ref': 'main'}

    project.branches.create(branch_args)
    

def updateSubmodule(project, submodule, branch, commit_sha, message) -> None:
    """
    Updates a submodule for a given project
    :param project: The project to perform the action
    :param submodule: The submodule to update
    :param branch: The branch to update the submodule on
    :param commit_sha: The commit sha of the submodule to update
    :param message: The commit message
    """
    project.update_submodule(
            submodule = submodule,
            branch = branch,
            commit_sha = commit_sha,
            commit_message = message)


def createMergeRequest(project, source_branch, target_branch, mr_title, mr_description) -> None:
    """
    Creates a merge request on a given project.
    :param project: The project to perform the action
    :param source_branch: The source branch of the merge request
    :param target_branch: The target branch of the merge request
    :param mr_title: The title of the merge request
    """
    mr_args = {'source_branch': source_branch,
               'target_branch': target_branch,
               'title': mr_title,
               'description': mr_description,
               'remove_source_branch': True}

    project.mergerequests.create(mr_args)
    

def deleteBranch(project, branch_name):
    """
    Deletes branch from project.
    :param project: The project to perform the action on
    :param branch_name: The branch to perform the action on
    """
    project.branches.delete(branch_name)


def createSubmoduleMR(parent_project, commit_id, commit_title) -> None:
    """
    Updates a project's submodule and creates a merge request. Cleans and exits program on error.
    :param parent_project: The project to perform the action on
    :param commit_id: The commit_sha of the submodule to update
    :param commit_title: The message of the latest submodule commit
    """
    issue_id = createIssue(parent_project, commit_id, commit_title)
    branch_name = f'{issue_id}-update-sdk-common-submodule-{commit_id[:8]}'
    createBranch(parent_project, branch_name)
    
    description = f'#{issue_id} Update umaa-sdk-common submodule to commit_sha: {commit_id[:8]}'
    
    mr_title = f'#{issue_id} AUTO: Update umaa-sdk-common submodule {commit_id[:8]}'
    mr_description = f"""\
**THIS MERGE REQUEST WAS AUTO-GENERATED**

Updates umaa-sdk-common to commit sha: {commit_id}

Branch name: \"{commit_title}\""""
    
    try:
        updateSubmodule(parent_project, 'umaa-sdk-common', branch_name, commit_id, description)
        createMergeRequest(parent_project, branch_name, 'main', mr_title, mr_description)
    except Exception as e:
        print(f"[ERROR] Unable to update submodule for \"{parent_project.name}\" due to exception:\n\t{e}")
        print("Closing Issue..")
        closeIssue(parent_project, issue_id)
        print("Removing branch..")
        deleteBranch(parent_project, branch_name)
        return
    
    print(f"Successfully created merge request for {parent_project.name}\n")


if __name__ == "__main__":

    parser = argparse.ArgumentParser(prog="UpdateParentProjects.py")
    parser.add_argument("-u", "--url", help="The URL of the Gitlab Server", required=True)  # ex. https://gitlab.com
    parser.add_argument("-t", "--token", help="The Gitlab token for authentication", required=True)
    args = parser.parse_args()

    gitlab_server = gitlab.Gitlab(url=args.url, private_token=args.token)

    sdk_url_base = 'peo-usc/pms406/rail/projects/umaa/umaa-sdk'
    sdk_common_url = f'{sdk_url_base}/umaa-sdk-common'
    mem_url = f'{sdk_url_base}/umaa-mission-execution-manager' # Mission Execution Manager Project
    roe_url = f'{sdk_url_base}/umaa-route-objective-executor'  # Route Objective Executor Project
    c2_sim_url = f'{sdk_url_base}/umaa-c2-sim'  # C2 Sim Project
    
    urls_list = [mem_url, roe_url, c2_sim_url]
    
    sdk_common_project = gitlab_server.projects.get(sdk_common_url)
    # Returns a tuple of the latest (commit_id, commit_message)
    commit_info = getLatestCommit(sdk_common_project, 'main')

    projects = [gitlab_server.projects.get(url)for url in urls_list] 
    
    for project in projects:
        createSubmoduleMR(project, commit_info.id, commit_info.title)